#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <memory>
#include <thread>
#include <vector>

#include "httpparser/httprequestparser.h"
#include "httpparser/request.h"
#include "my_exception.hpp"
#define HTTP_LENGTH 65535
class thread_info {
 public:
  const int unique_id;
  const int client_fd;
  thread_info(int id, int fd) : unique_id(id), client_fd(fd) {}
};

class proxy {
 private:
  const int listener;

 public:
  proxy() : listener(create_tcp_listener_fd("12345")) {}

 private:
  void start() {
    int id = 0;
    while (1) {
      int new_fd = proxy_accpect_connection();
      if (new_fd != -1) {
        std::unique_ptr<thread_info> t_info_ptr(new thread_info(id, new_fd));
        std::thread processing(process_request, std::move(t_info_ptr));
        processing.detach();
      }
      else {
        //log
      }
    }
  }

  int proxy_accpect_connection() {
    struct sockaddr_storage client_socket_addr;
    socklen_t len = sizeof(client_socket_addr);
    int new_fd = accept(this->listener, (struct sockaddr *)&client_socket_addr, &len);
    if (new_fd == -1) {
      //log this error
    }
    return new_fd;
  }

  /**
     thread function that process each incoming request
   */
  static void process_request(std::unique_ptr<thread_info> th_info) {
    int fd = th_info->client_fd;
    std::vector<char> buffer(HTTP_LENGTH, 0);
    int len_received = recv(fd, &buffer.data()[0], HTTP_LENGTH, 0);
    if (len_received <= 0) {
      //log error
      return;  //thread end
    }
    buffer.resize(len_received);
    std::unique_ptr<httpparser::Request> request_res_ptr = http_request_parse(buffer);
    std::string request_method = request_res_ptr->method;
    if (request_method == "CONNECT") {
      //do some crazy stuff about connect
    }
    else if (request_method == "POST") {
    }
    else if (request_method == "GET") {
    }
    else {
      //log
      //bad request 400
      return;  //process end
    }
  }

  static std::unique_ptr<httpparser::Request> http_request_parse(
      std::vector<char> & buffer) {
    httpparser::HttpRequestParser parser;
    std::unique_ptr<httpparser::Request> request_res_ptr(new httpparser::Request());
    httpparser::HttpRequestParser::ParseResult res = parser.parse(
        *request_res_ptr, &buffer.data()[0], &buffer.data()[0] + buffer.size());
    if (res == httpparser::HttpRequestParser::ParsingCompleted) {  //SUCCESS
      return request_res_ptr;  //return local value, no need to move()
    }
    else {
      //log this error
      //return a 400 error code.
      throw my_exception("cannot parse request");
    }
  }

  int create_tcp_listener_fd(const char * port) {  //reference from Beej's Guide
    int listener_fd = -1;
    int status = -1;
    struct addrinfo host_info_hints;
    struct addrinfo * host_info_list;
    struct addrinfo * host_ptr;

    memset(&host_info_hints, 0, sizeof(host_info_hints));
    host_info_hints.ai_family = AF_UNSPEC;
    host_info_hints.ai_socktype = SOCK_STREAM;  //TCP
    host_info_hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, port, &host_info_hints, &host_info_list);
    if (status != 0) {
      throw my_exception("cannot get addrinfo");
    }

    for (host_ptr = host_info_list; host_ptr != NULL; host_ptr = host_ptr->ai_next) {
      listener_fd =
          socket(host_ptr->ai_family, host_ptr->ai_socktype, host_ptr->ai_protocol);
      if (listener_fd == -1) {
        continue;
      }
      int yes = 1;
      setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
      status = bind(listener_fd, host_ptr->ai_addr, host_ptr->ai_addrlen);
      if (status == -1) {
        continue;
      }
      break;  //successful create socket and bind
    }

    freeaddrinfo(host_info_list);
    if (host_ptr == NULL) {
      throw my_exception("cannot create socket");
    }

    status = listen(listener_fd, 20);
    if (status == -1) {
      throw my_exception("cannot listen");
    }

    return listener_fd;
  }
};
