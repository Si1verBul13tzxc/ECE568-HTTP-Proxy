#include "time_format.hpp"
//time(0) and the input time will have an 1 hour deviation, probably because of daylight saving time?
time_t time_format::to_tm_format(std::string str){
    struct tm time_structure;
    size_t findGMT = str.find("GMT");
    str = str.substr(0, findGMT - 1);
    strptime(str.c_str(), "%A, %d %B %Y %H:%M:%S", &time_structure);
    time_t time_sec = mktime(&time_structure);
    // time_t now = time(0);
    // char buffer [80];
    // struct tm * timeinfo = localtime(&now);
    // strftime(buffer,80,"%A, %d %B %Y %H:%M:%S",timeinfo);
    // std::cout << time_sec << "   " << now;
    return time_sec;
}
