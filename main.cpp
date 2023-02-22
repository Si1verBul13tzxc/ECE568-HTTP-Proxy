#include <cstdlib>

#include "proxy.hpp"
int main() {
  //become a daemon
  //be_daemon()
  int listener = proxy::proxy_init_listener();
  proxy::cache = new Cache();

  if (listener == -1) {
    return EXIT_FAILURE;  //server start fails
  }
  proxy::start(listener);
  return EXIT_SUCCESS;  //never reach this
}
