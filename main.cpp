#include "proxy.hpp"

#include <cstdlib>
int main() {
  //become a daemon
  //be_daemon()
  while (1) {
    try {
      proxy p;
    }
    catch (my_exception & e) {
      //dowhatever
    }
  }
  return EXIT_SUCCESS;  //never reach this
}
