#include "time_format.hpp"
    
std::map<std::string, int> time_format::cor_map = {std::pair<std::string, int>("Jan", 0),
                                                    std::pair<std::string, int>("Feb", 1),
                                                    std::pair<std::string, int>("Mar", 2),
                                                    std::pair<std::string, int>("Apr", 3),
                                                    std::pair<std::string, int>("May", 4),
                                                    std::pair<std::string, int>("Jun", 5),
                                                    std::pair<std::string, int>("Jul", 6),
                                                    std::pair<std::string, int>("Aug", 7),
                                                    std::pair<std::string, int>("Sep", 8),
                                                    std::pair<std::string, int>("Oct", 9),
                                                    std::pair<std::string, int>("Nov", 10),
                                                    std::pair<std::string, int>("Dec", 11),
                                                    std::pair<std::string, int>("Sun", 0),
                                                    std::pair<std::string, int>("Mon", 1),
                                                    std::pair<std::string, int>("Tue", 2),
                                                    std::pair<std::string, int>("Wed", 3),
                                                    std::pair<std::string, int>("Thu", 4),
                                                    std::pair<std::string, int>("Fri", 5),
                                                    std::pair<std::string, int>("Sat", 6)
                                                    };

int time_format::get_correspondence(std::string str){
    return cor_map.find(str)->second;
}

time_t time_format::to_tm_format(std::string str){
    struct tm time_structure;
    size_t comma = str.find(",");
    std::string get_time = str.substr(comma + 2);
    //the time format now should be 03 Aug 2020 20:20:20 GMT
    time_structure.tm_mday = atoi(get_time.substr(0, 2).c_str());
    time_structure.tm_mon = get_correspondence(get_time.substr(3, 3).c_str());
    time_structure.tm_year = atoi(get_time.substr(7, 4).c_str()) - 1970;
    time_structure.tm_hour = atoi(get_time.substr(12, 2).c_str());
    time_structure.tm_min = atoi(get_time.substr(15, 2).c_str());
    time_structure.tm_sec = atoi(get_time.substr(18, 2).c_str());
    time_structure.tm_wday = get_correspondence(str.substr(0, comma).c_str());
    //add it to avoid error
    time_structure.tm_isdst = 0;
    time_t time_sec = mktime(&time_structure);
    return time_sec;
}
