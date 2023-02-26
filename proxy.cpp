#include "proxy.hpp"
std::ofstream log_f("/var/log/erss/proxy.log");
std::mutex mtx;

Cache * proxy::cache = NULL;
int proxy::proxy_init_listener() {
  int listener = socket_method::create_tcp_listener_fd("12345");
  return listener;
}

void proxy::start(int listener) noexcept {
  int id = 0;
  std::cout << "start" << std::endl;
  while (1) {
    try {
      std::string request_ip;
      int new_fd = socket_method::accpect_connection(listener, request_ip);
      if (new_fd != -1) {
        std::unique_ptr<thread_info> t_info_ptr(new thread_info(id, new_fd, request_ip));
        id++;
        std::thread processing(process_request, std::move(t_info_ptr));
        if (processing.joinable()) {  //detach may throw if not joinable
          processing.detach();
        }
      }
      else {
        log_id(-1, "ERROR accept new connection failed");
      }
    }
    catch (std::exception & e) {  //new may fail
      continue;
    }
  }
}

/**
     thread function that process each incoming request, 
     should try catch or exception and no throw
   */
void proxy::process_request(std::unique_ptr<thread_info> th_info) noexcept {
  int unique_id = th_info->unique_id;
  try {
    sock client_fd(th_info->client_fd);  //fd can be closed by destructor
    int fd = th_info->client_fd;
    std::vector<char> buffer(HTTP_LENGTH, 0);
    int len_received = recv(fd, &buffer.data()[0], HTTP_LENGTH, 0);
    if (len_received <= 0) {
      log_id(th_info->unique_id, "ERROR no request received");
      return;  //thread end
    }
    buffer.resize(len_received);
    std::unique_ptr<httpparser::Request> request_res_ptr =
        parser_method::http_request_parse(buffer);
    log_new_request(unique_id, th_info->request_ip, *request_res_ptr);
    std::string request_method = request_res_ptr->method;
    if (request_method == "CONNECT") {
      http_connect(std::move(th_info), std::move(request_res_ptr));
      log_id(unique_id, "Tunnel closed");
      debug_print("Tunnel closed");
    }
    else if (request_method == "POST") {
      //log for post message
      http_post(buffer, &len_received, std::move(th_info), std::move(request_res_ptr));
    }
    else if (request_method == "GET") {
      http_get(th_info.get(), request_res_ptr.get(), buffer);
    }
    else {  //proxy do not support this method
      log_id(th_info->unique_id, "ERROR method not supported");
      return;  //thread end
    }
  }
  catch (my_exception & e) {
    debug_print(e.what());
    log_id(unique_id, "ERROR " + std::string(e.what()));
  }
  catch (std::exception & e) {
    debug_print(e.what());
    log_id(-1, "ERROR (unexpected) " + std::string(e.what()));
  }
}

void proxy::http_get(thread_info * th_info,
                     httpparser::Request * request_res_ptr,
                     std::vector<char> & request_buffer) {
  debug_print("get request");
  time_t request_time = time(0);
  std::string request_uri =
      request_res_ptr->uri;  //TODO: may need to replace this with uri+hostname as key
  httpparser::Response * response = cache->get_response(request_uri);
  if (response == NULL) {
    debug_print("did not find in cache");
    get_from_server(request_buffer, th_info, request_res_ptr, &request_time);
  }else if (!is_fresh(response)){
    debug_print("the response in cache is expired");
    get_from_server(request_buffer, th_info, request_res_ptr, &request_time);
  }
  else {
    send_from_cache(response, th_info->client_fd);
  }
}

void proxy::send_from_cache(httpparser::Response * response, int client_fd) {
  debug_print("find in cache");
  std::string response_data = response->inspect();
  debug_print(response_data.c_str());
  std::vector<char> buffer(response_data.begin(), response_data.end());
  int size = buffer.size();
  int res = socket_method::sendall(client_fd, &buffer.data()[0], &size);
  //log file for the response message?
  if (res == -1) {  //send fail
    throw my_exception("fail to send all bytes");
  }
}

void proxy::get_from_server(std::vector<char> & request_buffer,
                            thread_info * th_info,
                            httpparser::Request * http_request,
                            time_t * request_time) {
  std::string get_port;
  std::string get_hostname;
  parser_method::get_host_and_port(*http_request, get_hostname, get_port);
  //Connect to server
  int server_fd = socket_method::connect_to_host(get_hostname.c_str(), get_port.c_str());
  sock fd_guard(server_fd);
  if (server_fd == -1) {
    throw my_exception("Cannot connect to server");
  }
  //send request message to server
  int size = request_buffer.size();
  socket_method::sendall(server_fd, (char *)&request_buffer.data()[0], &size);
  //receive the message from server
  std::vector<char> response_buffer(HTTP_LENGTH, 0);
  int response_length = recv(server_fd, &response_buffer.data()[0], HTTP_LENGTH, 0);
  time_t response_time = time(0);
  if (response_length <= 0) {
    //502
    throw my_exception("server problem");
  }
  else {
    //store the response in cache
    debug_print((std::string("store the reponse to cache") + http_request->uri).c_str());
    std::unique_ptr<httpparser::Response> response_res_ptr =
        parser_method::http_response_parse(response_buffer);
    //add request time and response time in the response header
    httpparser::Response::HeaderItem req_item = {"request_time", std::to_string(*request_time)}; 
    httpparser::Response::HeaderItem resp_item = {"response_time", std::to_string(response_time)}; 
    response_res_ptr->headers.push_back(req_item);
    response_res_ptr->headers.push_back(resp_item);
    proxy::cache->add_response(http_request->uri, std::move(response_res_ptr));
    //send the response to client
    int res = socket_method::sendall(
        th_info->client_fd, (char *)&response_buffer.data()[0], &response_length);
    //log file for the response message
    if (res == -1) {
      throw my_exception("fail to send all bytes");
    }
  }
}

/** 
    Follow rfc7234 4.2. Freshness
    response_is_fresh = (freshness_lifetime > current_age);
    use calculate_age defined below to calculate current_age.
*/
bool proxy::is_fresh(httpparser::Response * response) {
  return calculate_freshness_lifetime(response) > calculate_age(response);
}

/**
 * max-age
 * expire header 
*/

/** 
    rfc 4.2.1, and heuristic freshness: if need, use 0 as its value for simplicity
    if cache is shared and s_maxage is present, use s_maxage
    if max-age present, use max-age
    if expires response header filed is present, use its value - the value of the date response header field
    if no explicit expiration time , go to heiristic freshness 4.2.2
*/
long proxy::calculate_freshness_lifetime(httpparser::Response * response) {
  std::string cache_control = parser_method::response_get_header_value(*response, "Cache-Control");
  std::string expires;
  size_t s_maxage;
  size_t max_age;
  size_t ends;
  std::string s_maxage_str;
  std::string max_age_str;
  time_t freshness;
  if ((s_maxage = cache_control.find("s-maxage")) != std::string::npos) {
    if((ends = cache_control.substr(s_maxage).find(",")) != std::string::npos){
      s_maxage_str = cache_control.substr(s_maxage + 9, ends - 9);
    }else{
      s_maxage_str = cache_control.substr(s_maxage + 9);
    }
    std::string::size_type sz;
    freshness = std::stol(s_maxage_str, &sz);
  }else if((max_age = cache_control.find("max-age")) != std::string::npos) {
    if((ends = cache_control.substr(max_age).find(",")) != std::string::npos){
      max_age_str = cache_control.substr(max_age  + 8, ends - 8);
    }else{
      max_age_str = cache_control.substr(max_age + 8);
    }
    std::string::size_type sz;
    freshness = std::stol(max_age_str, &sz);
  }else if((expires = parser_method::response_get_header_value(*response, "expires")) != "") {
    time_t expire_time = time_format::to_tm_format(expires);
    std::string date_str = parser_method::response_get_header_value(*response, "Date");
    time_t date = time_format::to_tm_format(date_str);
    freshness = expire_time - date;
  }else{
    //go to heuristics freshness
    debug_print("calculate freshness lifetime encounter unexpected senario");
    return -1;
  }
  return freshness;
}

/** 
    rfc 4.2.3.
    may need to add some new attribute in httpparser::response to store values like response_time, request_time
    age_value(Age header field), date_value(Date header field), now(the current value of the clock at the host 
    performing the calculation), request_time(the current time when making request), response_time (the current 
    time of receiving response)
    calculate ways: 1. apparent_age: response_time - date_value, if negative replace with 0
    2. corrected_age_value: iif all cache in response implement HTTP/1.1, cache should interpret this value from 
    the request was initiated, not the time the response was received
     = max(apparent_age, corrected_age_value), where c_a_v = age_value + response_delay
     if the cache is confident, c_a_v can be used as c_i_a
     current_age of a stored response can be calculated by c_i_a + resident_time, where resident_time = now - response_time;
*/

long proxy::calculate_age(httpparser::Response * response) {
  long corrected_init_age = std::max(apparent_age(response), corrected_age_value(response));
  time_t now = time(0);
  std::string response_time_str = parser_method::response_get_header_value(*response, "response_time");
  std::string::size_type sz;
  time_t response_time = std::stol(response_time_str, &sz);
  int current_age = corrected_init_age + now - response_time;
  return current_age;
}

/***
other helper method you need
 **/

long proxy::corrected_age_value(httpparser::Response * response){
  std::string age_value_str = parser_method::response_get_header_value(*response, "age");
  std::string response_time_str = parser_method::response_get_header_value(*response, "response_time");
  std::string request_time_str = parser_method::response_get_header_value(*response, "request_time");;
  std::string::size_type sz1, sz2, sz3;
  time_t request_time = std::stol(request_time_str, &sz1);
  time_t response_time = std::stol(response_time_str, &sz2);
  time_t age_value = std::stol(age_value_str, &sz3);
  time_t response_delay = response_time - request_time;
  time_t corrected_age_value = age_value + response_delay;
  return corrected_age_value;
}

long proxy::apparent_age(httpparser::Response * response){
  std::string date_value_str = parser_method::response_get_header_value(*response, "Date");
  std::string response_time_str = parser_method::response_get_header_value(*response, "response_time");
  std::string::size_type sz;
  time_t response_time = std::stol(response_time_str, &sz);
  time_t date_value = time_format::to_tm_format(date_value_str);
  return std::max((long)0, response_time - date_value);
}

/** 
helper method ends
*/

void proxy::http_connect(std::unique_ptr<thread_info> th_info,
                         std::unique_ptr<httpparser::Request> http_request) {
  std::string hostname;
  std::string port;
  parser_method::get_host_and_port(*http_request, hostname, port);
  int uri_fd = socket_method::connect_to_host(hostname.c_str(), port.c_str());
  if (uri_fd == -1) {
    std::string message("connect " + hostname + ":" + port + " fails");
    throw my_exception(message.c_str());
  }
  sock uri_fd_guard(uri_fd);
  http_send_200ok(th_info->client_fd, th_info->unique_id);
  http_connect_forward_messages(uri_fd, *th_info);
}

void proxy::http_send_200ok(int client_fd, int unique_id) {
  const char * ok = "HTTP/1.1 200 OK\r\n\r\n";
  std::string msg = "Responding \"" + std::string(ok) + "\"";
  log_id(unique_id, msg);
  debug_print((std::to_string(unique_id) + msg).c_str());
  send(client_fd, ok, strlen(ok), 0);
}

void proxy::http_connect_forward_messages(int uri_fd, const thread_info & info) {
  //forwarding message from one end to another
  struct pollfd pollfds[2];
  pollfds[0].fd = info.client_fd;
  pollfds[0].events = POLLIN;
  pollfds[1].fd = uri_fd;
  pollfds[1].events = POLLIN;
  while (1) {
    int pollcount = poll(pollfds, 2, -1);
    if (pollcount == -1) {
      throw my_exception("poll fails on forwarding message between client and server");
    }
    for (int i = 0; i < 2; i++) {
      if (pollfds[i].revents & POLLIN) {
        std::vector<char> buffer(HTTP_LENGTH, 0);
        int len_received = recv(pollfds[i].fd, &buffer.data()[0], HTTP_LENGTH, 0);
        if (len_received <= 0) {  //disconnect
          return;
        }
        buffer.resize(len_received);
        int res = socket_method::sendall(
            pollfds[i ^ 1].fd, (char *)&buffer.data()[0], &len_received);
        if (res == -1) {  //send fail
          throw my_exception("fail to send all bytes");
        }
      }
    }
  }
}

void proxy::http_post(std::vector<char> & request_buffer,
                      int * len_received,
                      std::unique_ptr<thread_info> th_info,
                      std::unique_ptr<httpparser::Request> http_request) {
  debug_print("post method");
  std::string post_port;
  std::string post_hostname;
  parser_method::get_host_and_port(*http_request, post_hostname, post_port);
  //Connect to the post server
  int server_fd =
      socket_method::connect_to_host(post_hostname.c_str(), post_port.c_str());
  if (server_fd == -1) {
    throw my_exception("post:cannot connect to server");
  }
  sock server_fd_guard(server_fd);
  //send request message to server
  int res =
      socket_method::sendall(server_fd, (char *)&request_buffer.data()[0], len_received);
  if (res == -1) {
    throw my_exception("post fail to send all bytes");
  }
  //receive the message from server
  std::vector<char> response_buffer(HTTP_LENGTH, 0);
  int response_length = recv(server_fd, &response_buffer.data()[0], HTTP_LENGTH, 0);
  if (response_length <= 0) {
    throw my_exception("post fail to recv from server");
  }
  else {
    std::string msg = "The response mes length is: " + std::to_string(response_length);
    debug_print(msg.c_str());
    int res = socket_method::sendall(
        th_info->client_fd, (char *)&response_buffer.data()[0], &response_length);
    if (res == -1) {  //send fail
      throw my_exception("fail to send all bytes");
    }
  }
}

/*************************
Below are some helper functions
***************************/

void proxy::debug_print(const char * msg) {
  if (DEBUG) {
    std::cout << msg << std::endl;
  }
}

const char * proxy::get_current_time() {
  time_t time_raw;
  struct tm * time_info;
  time(&time_raw);
  time_info = gmtime(&time_raw);
  return asctime(time_info);
}

/** 
    write message to the log file
    @param id: -1 means no-id
*/
void proxy::log_id(int id, std::string msg) {
  mtx.lock();
  if (id != -1) {
    log_f << id << ": " << msg << std::endl;
  }
  else {
    log_f << "no-id: " << msg << std::endl;
  }
  mtx.unlock();
}

void proxy::log_new_request(int unique_id, std::string ip, httpparser::Request & req) {
  //log new request
  std::string msg = std::to_string(unique_id) + ": \"" +
                    parser_method::get_request_line(req) + "\" from " + ip + " @ " +
                    get_current_time();
  log_id(unique_id, msg);
  debug_print(msg.c_str());
}
