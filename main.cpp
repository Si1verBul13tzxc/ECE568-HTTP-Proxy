#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>
#include "proxy.hpp"

int main() {
  //become a daemon
  pid_t pid = getpid();
  if ((pid = fork()) < 0){
    std::cerr << "fork fail";
    exit(1);
  }
  else if (pid > 0){ /* parent */
    exit(0);
  }
  /* proxy child A */
  setsid();
  if ((pid = fork()) < 0){
      std::cerr << "fork fail";
      exit(1);
  }
  else if (pid != 0){  
      exit(0);
  }
  
  int listener = proxy::proxy_init_listener();
  proxy::cache = new Cache(100);

  if (listener == -1) {
    return EXIT_FAILURE;  //server start fails
  }
  proxy::start(listener);
  close(listener);
  return EXIT_SUCCESS;  //never reach this
}
