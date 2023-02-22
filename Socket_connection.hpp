#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <string>

#include "my_exception.hpp"

namespace socket_method {
  int sendall(int sock_fd, char * buf, int * len);
  int create_tcp_listener_fd(const char * port);
  int connect_to_host(const char * theHostname, const char * thePort);
  int accpect_connection(int listener, std::string & request_ip);
  void * get_in_addr(struct sockaddr * sa);
}  // namespace socket_method
