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
      debug_print(e.what());
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
    sock client_fd_guard(th_info->client_fd);  //fd can be closed by destructor
    int fd = th_info->client_fd;
    std::vector<char> buffer(HTTP_LENGTH, 0);
    int len_received = recv(fd, &buffer.data()[0], HTTP_LENGTH, 0);
    if (len_received <= 0) {
      debug_print("no request reveived");
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
      http_post(buffer, std::move(th_info), std::move(request_res_ptr));
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

/** 
    HTTP GET
*/
void proxy::http_get(thread_info * th_info,
                     httpparser::Request * request,
                     std::vector<char> & request_buffer) {
  debug_print("GET request");
  httpparser::Response * response = cache->get_response(request->uri);
  if (response == NULL) {
    debug_print("did not find in cache");
    get_from_server(request_buffer, th_info, request);
  }
  else {
    debug_print("in cache");
    if (response_need_validate(response) || request_require_validate(request)) {
      debug_print("need revalidation");
      validation(th_info, request, response);
    }
    else {
      debug_print("no need to revalidation");
      send_response_in_cache(response, th_info->client_fd);
    }
  }
}

/** 
    HTTP POST
*/
void proxy::http_post(std::vector<char> & request_buffer,
                      std::unique_ptr<thread_info> th_info,
                      std::unique_ptr<httpparser::Request> http_request) {
  debug_print("POST method");
  int server_fd = connect_to_server(http_request.get());
  sock server_fd_guard(server_fd);
  std::vector<char> response_buffer(HTTP_LENGTH, 0);
  int response_length = one_round_trip(request_buffer, server_fd, response_buffer);
  std::string msg = "The response mes length is: " + std::to_string(response_length);
  debug_print(msg.c_str());
  send_to(th_info->client_fd, response_buffer);
}

/** 
    HTTP CONNECT
*/
void proxy::http_connect(std::unique_ptr<thread_info> th_info,
                         std::unique_ptr<httpparser::Request> http_request) {
  int uri_fd = connect_to_server(http_request.get());
  sock uri_fd_guard(uri_fd);
  http_send_200ok(th_info->client_fd, th_info->unique_id);
  http_connect_forward_messages(uri_fd, *th_info);
}

/*************************
Below are some helper functions
***************************/

void proxy::send_response_in_cache(httpparser::Response * response, int client_fd) {
  debug_print("send the response in cache");
  std::string response_data = response->inspect();
  debug_print(response_data.c_str());
  std::vector<char> buffer(response_data.begin(), response_data.end());
  send_to(client_fd, buffer);
}

void proxy::get_from_server(std::vector<char> & request_buffer,
                            thread_info * th_info,
                            httpparser::Request * http_request) {
  int server_fd = connect_to_server(http_request);
  sock fd_guard(server_fd);
  std::vector<char> response_buffer(HTTP_LENGTH, 0);
  int resp_len = one_round_trip(request_buffer, server_fd, response_buffer);
  assert(resp_len == (int)response_buffer.size());
  if (is_chunked(response_buffer)) {
    chunked_transfer(th_info, server_fd, response_buffer);
    return;
  }
  std::unique_ptr<httpparser::Response> response_res_ptr =
      parser_method::http_response_parse(response_buffer);
  if (response_may_store(response_res_ptr.get()) &&
      !request_no_store(http_request)) {  //store the response in cache if can store
    debug_print((std::string("store the reponse to cache") + http_request->uri).c_str());
    response_res_ptr->response_time = time(0);
    cache->add_response(http_request->uri, std::move(response_res_ptr));
  }
  else {
    debug_print("cannot store this response in cache");
  }
  send_to(th_info->client_fd, response_buffer);
}

bool proxy::is_chunked(std::vector<char> & response_buffer) {
  std::string msg(response_buffer.begin(), response_buffer.end());
  size_t pos = msg.find("chunked");
  return pos != std::string::npos;
}

void proxy::chunked_transfer(thread_info * th_info,
                             int server_fd,
                             std::vector<char> & first_buffer) {
  send_to(th_info->client_fd, first_buffer);  //send first response
  debug_print("chunked_transfer");
  std::vector<char> response_buffer(HTTP_LENGTH, 0);
  while (1) {
    response_buffer.resize(HTTP_LENGTH);
    int resp_len = recv(server_fd, &response_buffer.data()[0], HTTP_LENGTH, 0);
    if (resp_len <= 0) {
      debug_print("chunked thransfer finished");
      break;
    }
    response_buffer.resize(resp_len);
    send_to(th_info->client_fd, response_buffer);
  }
}

/** 
    Connect to server, the returned should be passed to an sock object
 */
int proxy::connect_to_server(httpparser::Request * http_request) {
  //Connect to server
  std::string get_port;
  std::string get_hostname;
  parser_method::get_host_and_port(*http_request, get_hostname, get_port);
  int server_fd = socket_method::connect_to_host(get_hostname.c_str(), get_port.c_str());
  if (server_fd == -1) {
    throw my_exception("Cannot connect to server");
  }
  return server_fd;
}

/** 
    Reference: RFC7234 3.
    Return true indicates this response can be stored into cache.
*/
bool proxy::response_may_store(httpparser::Response * response) {
  if (response->statusCode != 200) {
    return false;
  }
  std::string cache_control =
      parser_method::response_get_header_value(*response, "Cache-Control");
  if (cache_control.find("no-store") != std::string::npos ||
      cache_control.find("private") != std::string::npos) {
    return false;
  }
  if (cache_control != "" ||
      parser_method::response_get_header_value(*response, "Expires") != "" ||
      parser_method::response_get_header_value(*response, "ETag") != "" ||
      parser_method::response_get_header_value(*response, "Last-Modified") != "" ||
      cache_control.find("max-age") != std::string::npos ||
      cache_control.find("s-maxage") != std::string::npos ||
      cache_control.find("public") != std::string::npos) {  //add last modified and etag
    return true;
  }
  return false;
}

/** 
    Return false if no need to validate
*/
bool proxy::response_need_validate(httpparser::Response * response) {
  if (!is_fresh(response)) {  //response is stale
    debug_print("stale response");
    return true;
  }
  std::string cache_control =
      parser_method::response_get_header_value(*response, "Cache-Control");
  if (cache_control.find("no-cache") != std::string::npos) {  //it is fresh, but no-cache
    return true;
  }
  return false;
}

/** 
    Return ture if the request require validate
*/
bool proxy::request_require_validate(httpparser::Request * req) {
  std::string cache_control =
      parser_method::request_get_header_value(*req, "Cache-Control");
  if (cache_control.find("no-cache") != std::string::npos) {
    return true;
  }
  return false;
}

/** 
    True if this request require its response to be no-store
*/
bool proxy::request_no_store(httpparser::Request * req) {
  std::string cache_control =
      parser_method::request_get_header_value(*req, "Cache-Control");
  if (cache_control.find("no-store") != std::string::npos) {
    return true;
  }
  return false;
}

void proxy::validation(thread_info * th_info,
                       httpparser::Request * request,
                       httpparser::Response * response) {
  int server_fd = connect_to_server(request);
  sock server_fd_guard(server_fd);
  httpparser::Request conditional_request;
  construct_conditional_request(request, response, conditional_request);
  debug_print(conditional_request.inspect().c_str());
  std::string request_msg = conditional_request.inspect();
  std::vector<char> conditional_request_buffer(request_msg.begin(), request_msg.end());
  std::vector<char> new_response(HTTP_LENGTH, 0);
  int resp_length = one_round_trip(conditional_request_buffer, server_fd, new_response);
  assert(resp_length == (int)new_response.size());
  std::unique_ptr<httpparser::Response> new_response_parsed =
      parser_method::http_response_parse(new_response);
  unsigned int code = new_response_parsed->statusCode;
  if (code == 304) {  //not modified
    debug_print("receive 304 not modified on validation");
    freshen_headers(response, new_response_parsed.get());
    response->response_time = time(0);
    send_response_in_cache(response, th_info->client_fd);
    return;
  }
  else if (code == 200) {  //modified, replace old one in cache
    new_response_parsed->response_time = time(0);
    cache->add_response(request->uri, std::move(new_response_parsed));
    debug_print("receive 200 ok on validation");
  }
  else {  //5xx,forward this new_response to client
    log_id(th_info->unique_id, "NOTE server error for validtion");
    debug_print("server problem on revalidation");
  }
  send_to(th_info->client_fd, new_response);
}

/** 
    Freshening stored responses upon validtion
    @param response, the response in cache
    @param new_responsem, the response returned by validtion
*/
void proxy::freshen_headers(httpparser::Response * response,
                            httpparser::Response * new_response) {
  for (size_t i = 0; i < new_response->headers.size(); i++) {
    parser_method::update_response(
        *response, new_response->headers[i].name, new_response->headers[i].value);
  }
}

/** 
    Generate a conditional request
*/
void proxy::construct_conditional_request(httpparser::Request * request,
                                          httpparser::Response * response,
                                          httpparser::Request & conditional_request) {
  conditional_request.method = request->method;
  conditional_request.uri = request->uri;
  conditional_request.versionMajor = request->versionMajor;
  conditional_request.versionMinor = request->versionMinor;
  std::string host = parser_method::request_get_header_value(*request, "Host");
  struct httpparser::Request::HeaderItem item = {"Host", host};
  conditional_request.headers.push_back(item);
  std::string last_modified =
      parser_method::response_get_header_value(*response, "Last-Modified");
  if (last_modified != "") {
    struct httpparser::Request::HeaderItem item = {"If-Modified-Since", last_modified};
    conditional_request.headers.push_back(item);
  }
  std::string etag = parser_method::response_get_header_value(*response, "ETag");
  if (etag != "") {
    struct httpparser::Request::HeaderItem item = {"If-None-Match", etag};
    conditional_request.headers.push_back(item);
  }
}

time_t proxy::to_tm_format(std::string str) {
  struct tm time_structure;
  size_t findGMT = str.find("GMT");
  str = str.substr(0, findGMT - 1);
  strptime(str.c_str(), "%A, %d %B %Y %H:%M:%S", &time_structure);
  time_t time_sec = mktime(&time_structure);
  return time_sec;
}

/** 
    Follow rfc7234 4.2. Freshness
    response_is_fresh = (freshness_lifetime > current_age);
    use calculate_age defined below to calculate current_age.
*/
bool proxy::is_fresh(httpparser::Response * response) {
  debug_print("is_fresh?");
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
  debug_print("freshness lifetime");
  std::string cache_control =
      parser_method::response_get_header_value(*response, "Cache-Control");
  std::string expires;
  size_t s_maxage;
  size_t max_age;
  size_t ends;
  std::string s_maxage_str;
  std::string max_age_str;
  time_t freshness;
  if ((s_maxage = cache_control.find("s-maxage")) != std::string::npos) {
    if ((ends = cache_control.substr(s_maxage).find(",")) != std::string::npos) {
      s_maxage_str = cache_control.substr(s_maxage + 9, ends - 9);
    }
    else {
      s_maxage_str = cache_control.substr(s_maxage + 9);
    }
    std::string::size_type sz;
    freshness = std::stol(s_maxage_str, &sz);
  }
  else if ((max_age = cache_control.find("max-age")) != std::string::npos) {
    if ((ends = cache_control.substr(max_age).find(",")) != std::string::npos) {
      max_age_str = cache_control.substr(max_age + 8, ends - 8);
    }
    else {
      max_age_str = cache_control.substr(max_age + 8);
    }
    std::string::size_type sz;
    freshness = std::stol(max_age_str, &sz);
  }
  else if ((expires = parser_method::response_get_header_value(*response, "Expires")) !=
           "") {
    time_t expire_time = to_tm_format(expires);
    std::string date_str = parser_method::response_get_header_value(*response, "Date");
    time_t date = to_tm_format(date_str);
    freshness = expire_time - date;
  }
  else {
    //go to heuristics freshness
    freshness = 0;
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
  debug_print("calculate age");
  long corrected_init_age =
      std::max(apparent_age(response), corrected_age_value(response));
  time_t now = time(0);
  //std::string response_time_str =
  //    parser_method::response_get_header_value(*response, "response_time");
  //  std::string::size_type sz;
  //time_t response_time = std::stol(response_time_str, &sz);
  time_t response_time = response->response_time;
  long current_age = corrected_init_age + now - response_time;
  return current_age;
}

long proxy::corrected_age_value(httpparser::Response * response) {
  std::string age_value_str = parser_method::response_get_header_value(*response, "age");
  //std::string response_time_str =
  //    parser_method::response_get_header_value(*response, "response_time");
  //std::string request_time_str =
  //    parser_method::response_get_header_value(*response, "request_time");
  ;
  std::string::size_type /*sz1, sz2,*/ sz3;
  //if (response_time_str == "" || request_time_str == "") {
  //  std::string message("There is invalid time when calculating corrected_Age\n");
  //  throw my_exception(message.c_str());
  //}
  //time_t request_time = std::stol(request_time_str, &sz1);
  //time_t response_time = std::stol(response_time_str, &sz2);
  //time_t response_time = response->response_time;
  time_t age_value = age_value_str == "" ? (long)0 : std::stol(age_value_str, &sz3);
  //time_t response_delay = response_time - request_time;
  time_t corrected_age_value = age_value;  // + response_delay;
  return corrected_age_value;
}

long proxy::apparent_age(httpparser::Response * response) {
  std::string date_value_str =
      parser_method::response_get_header_value(*response, "Date");
  //std::string response_time_str =
  //    parser_method::response_get_header_value(*response, "response_time");

  //if (response_time_str == "" || date_value_str == "") {
  //std::string message("There is invalid time when calculating apparent_Age\n");
  //throw my_exception(message.c_str());
  //}
  //std::string::size_type sz;
  //time_t response_time = std::stol(response_time_str, &sz);
  time_t response_time = response->response_time;
  time_t date_value = to_tm_format(date_value_str);
  return std::max((long)0, response_time - date_value);
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

/** 
    send all bytes from buffer to destination, throw if fail
*/
void proxy::send_to(int fd, std::vector<char> & buffer) {
  int size = buffer.size();
  int res = socket_method::sendall(fd, (char *)&buffer.data()[0], &size);
  if (res == -1) {  //send fail
    throw my_exception("fail to send all bytes");
  }
}

/** 
    This function simply send all bytes to server, and receive all bytes from server
    @return the reveived byte length
    the response_buffer size is resized.
*/
int proxy::one_round_trip(std::vector<char> & request_buffer,
                          int server_fd,
                          std::vector<char> & response_buffer) {
  //send request message to server
  int size = request_buffer.size();
  int res = socket_method::sendall(server_fd, (char *)&request_buffer.data()[0], &size);
  if (res == -1) {
    throw my_exception("fail to send all bytes to server");
  }
  //receive the message from server
  int response_length = recv(server_fd, &response_buffer.data()[0], HTTP_LENGTH, 0);
  if (response_length <= 0) {
    throw my_exception("fail to recv from server");
  }
  response_buffer.resize(response_length);
  return response_length;
}

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
