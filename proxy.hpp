#include <assert.h>
#include <poll.h>
#include <time.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "Socket_connection.hpp"
#include "cache.hpp"
#include "parser_method.hpp"

#define HTTP_LENGTH 65535
#define DEBUG 1

class thread_info;

class proxy {
 public:
  static Cache * cache;
  static int proxy_init_listener();
  static void start(int listener) noexcept;
  static void debug_print(const char * msg);

 private:
  static void process_request(std::unique_ptr<thread_info> th_info) noexcept;
  static void http_connect(std::unique_ptr<thread_info> th_info,
                           std::unique_ptr<httpparser::Request> http_request);
  static void http_connect_forward_messages(int uri_fd, const thread_info & info);
  static void http_post(std::vector<char> & request_buffer,
                        std::unique_ptr<thread_info> th_info,
                        std::unique_ptr<httpparser::Request> http_request);
  static void log_id(int id, std::string msg);
  static void http_send_200ok(int client_fd, int unique_id);
  static const char * get_current_time();
  static void http_get(thread_info * th_info,
                       httpparser::Request * request_res_ptr,
                       std::vector<char> & request_buffer);
  static void get_from_server(std::vector<char> & request_buffer,
                              thread_info * th_info,
                              httpparser::Request * http_request);
  static void log_new_request(int unique_id, std::string ip, httpparser::Request & req);
  static void send_response_in_cache(httpparser::Response * response, int client_fd);
  static bool response_may_store(httpparser::Response * response);
  static bool response_need_validate(httpparser::Response * response);
  static void construct_conditional_request(httpparser::Request * request,
                                            httpparser::Response * response,
                                            httpparser::Request & conditional_request);
  static void freshen_headers(httpparser::Response * response,
                              httpparser::Response * new_response);
  static int connect_to_server(httpparser::Request * http_request);
  static int one_round_trip(std::vector<char> & request_buffer,
                            int server_fd,
                            std::vector<char> & response_buffer);
  static void send_to(int fd, std::vector<char> & buffer);
  static void validation(thread_info * th_info,
                         httpparser::Request * request,
                         httpparser::Response * response);
  static void chunked_transfer(thread_info * th_info,
                               int server_fd,
                               std::vector<char> & first_buffer);
  static bool is_chunked(std::vector<char> & response_buffer);
  static bool request_require_validate(httpparser::Request * req);
  static bool request_no_store(httpparser::Request * req);
  static time_t to_tm_format(std::string str);
  static bool is_fresh(httpparser::Response * response);
  static long calculate_freshness_lifetime(httpparser::Response * response);
  static long calculate_age(httpparser::Response * response);
  static long apparent_age(httpparser::Response * response);
  static long corrected_age_value(httpparser::Response * response);
};

class thread_info {
 public:
  const int unique_id;
  const int client_fd;
  const std::string request_ip;
  thread_info(int id, int fd, const std::string & ip) :
      unique_id(id), client_fd(fd), request_ip(ip) {}
  ~thread_info() { proxy::debug_print("destruct thread_info"); }
};

class sock {
 public:
  const int sock_fd;
  sock(int fd) : sock_fd(fd) {}
  ~sock() {
    proxy::debug_print("destruct close sock");
    close(sock_fd);
  }
};
