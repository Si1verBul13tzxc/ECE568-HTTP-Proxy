#include <assert.h>
#include <poll.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "Socket_connection.hpp"
#include "parser_method.hpp"
#define HTTP_LENGTH 65535

class thread_info {
 public:
  const int unique_id;
  const int client_fd;
  thread_info(int id, int fd) : unique_id(id), client_fd(fd) {}
};

class proxy {
 public:
  static int proxy_init_listener();
  static void start(int listener) noexcept;

 private:
  static void process_request(std::unique_ptr<thread_info> th_info) noexcept;
  static void http_connect(std::unique_ptr<thread_info> th_info,
                           std::unique_ptr<httpparser::Request> http_request);
  static void http_connect_forward_messages(int uri_fd, const thread_info & info);
  static void http_post(std::vector<char> request_buffer,
                        int * len_received,
                        std::unique_ptr<thread_info> th_info,
                        std::unique_ptr<httpparser::Request> http_request);
  static void log_id(int id, std::string msg);
  static void http_send_200ok(int client_fd);
};
