#include <assert.h>
#include <poll.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <time.h>
#include <chrono>
#include <map>

class time_format {
 public:
  static std::map<std::string, int> cor_map;

 public:

  /**
   * return the digital representation of months and weeks
  */
  static int get_correspondence(std::string str);

  /**
   * change the time format to the tm structure
  */
  static time_t to_tm_format(std::string str);

};