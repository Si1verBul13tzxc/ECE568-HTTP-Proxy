// #include <iostream>
// #include "httpparser/response.h"
// #include "httpparser/httpresponseparser.h"
// #include "parser_method.hpp"
// #include "time_format.hpp"

// long calculate_freshness_lifetime(httpparser::Response * response) {
//   std::string cache_control = parser_method::response_get_header_value(*response, "Cache-Control");
//   std::string expires;
//   size_t s_maxage;
//   size_t max_age;
//   size_t ends;
//   std::string s_maxage_str;
//   std::string max_age_str;
//   time_t freshness;
//   if ((s_maxage = cache_control.find("s-maxage")) != std::string::npos) {
//     if((ends = cache_control.substr(s_maxage).find(",")) != std::string::npos){
//       s_maxage_str = cache_control.substr(s_maxage + 9, ends - 9);
//     }else{
//       s_maxage_str = cache_control.substr(s_maxage + 9);
//     }
//     std::string::size_type sz;
//     std::cout << "get from s_maxage\n";
//     freshness = std::stol(s_maxage_str, &sz);
//   }else if((max_age = cache_control.find("max-age")) != std::string::npos) {
//     if((ends = cache_control.substr(max_age).find(",")) != std::string::npos){
//       max_age_str = cache_control.substr(max_age  + 8, ends - 8);
//     }else{
//       max_age_str = cache_control.substr(max_age + 8);
//     }
//     std::string::size_type sz;
//     std::cout << "get from max_age\n";
//     freshness = std::stol(max_age_str, &sz);
//   }else if((expires = parser_method::response_get_header_value(*response, "expires")) != "") {
//     time_t expire_time = time_format::to_tm_format(expires);
//     std::string date_str = parser_method::response_get_header_value(*response, "Date");
//     time_t date = time_format::to_tm_format(date_str);
//     freshness = expire_time - date;
//     std::cout << "get from expires\n";
//   }else{
//     //go to heuristics freshness
//     std::cout << "calculate freshness lifetime encounter unexpected senario";
//     return -1;
//   }
//   return freshness;
// }


// long corrected_age_value(httpparser::Response * response){
//   std::string response_time_str = parser_method::response_get_header_value(*response, "response_time");
//   std::string request_time_str = parser_method::response_get_header_value(*response, "request_time");;
//   std::string::size_type sz1, sz2, sz3;
//   std::string age_value_str = parser_method::response_get_header_value(*response, "age");
//   time_t response_time = std::stol(response_time_str, &sz2);
//   time_t request_time = std::stol(request_time_str, &sz1);
//   time_t age_value = std::stol(age_value_str, &sz3);
//   time_t response_delay = response_time - request_time;
//   time_t corrected_age_value = age_value + response_delay;
//   return corrected_age_value;
// }

// long apparent_age(httpparser::Response * response){
//   std::string date_value_str = parser_method::response_get_header_value(*response, "Date");
//   std::string response_time_str = parser_method::response_get_header_value(*response, "response_time");
//   std::string::size_type sz;
//   time_t response_time = std::stol(response_time_str, &sz);
//   time_t date_value = time_format::to_tm_format(date_value_str);
//   return std::max((long)0, response_time - date_value);
// }

// long calculate_age(httpparser::Response * response) {
//   long corrected_init_age = std::max(apparent_age(response), corrected_age_value(response));
//   time_t now = time(0);
//   std::string response_time_str = parser_method::response_get_header_value(*response, "response_time");
//   std::string::size_type sz;
//   time_t response_time = std::stol(response_time_str, &sz);
//   int current_age = corrected_init_age + now - response_time;
//   return current_age;
// }


// // long test_format() {
// //   std::string time = "Sun, 26 Feb 2023 17:00:00 GMT";
// //   time_t transfer = time_format::to_tm_format(time);
// //   return transfer;
// // }

// void test_calculate_freshness(){
//     const char text[] =
//             "HTTP/1.1 200 OK\r\n"
//             "Content-Length: 55\r\n"
//             "age: 324434232\r\n"
//             "Content-Type: text/html\r\n"
//             "Last-Modified: Wed, 12 Aug 1998 15:03:50 GMT\r\n"
//             "Accept-Ranges: bytes\r\n"
//             "ETag: “04f97692cbd1:377”\r\n"
//             "Date: Thu, 19 Jun 2008 19:29:07 GMT\r\n"
//             "Cache-Control: "
//             "request_time: Thu, 19 Jun 2022 19:29:07 GMT\r\n"
//             "expires: Thu, 19 Jun 2023 19:29:07 GMT\r\n"
//             "\r\n"
//             "<html />";

//     httpparser::Response response;
//     httpparser::HttpResponseParser parser;
//     httpparser::HttpResponseParser::ParseResult res = parser.parse(response, text, text + sizeof(text));
//     time_t request_time = time_format::to_tm_format("Sun, 26 Feb 2023 17:20:00 GMT");
//     time_t response_time = time(0);
//     //std::cout << request_time << "   " << response_time << "\n";
//     httpparser::Response::HeaderItem req_item = {"request_time", std::to_string(request_time)}; 
//     httpparser::Response::HeaderItem resp_item = {"response_time", std::to_string(response_time)}; 
//     response.headers.push_back(req_item);
//     response.headers.push_back(resp_item);
//     std::string response_time_str = parser_method::response_get_header_value(response, "response_time");
//     std::string request_time_str = parser_method::response_get_header_value(response, "request_time");
//     std::string::size_type sz1, sz2;
//     std::cout << response_time_str << "\n";
//     time_t request_t = std::stol(request_time_str, &sz1);
//     time_t response_t = std::stol(response_time_str, &sz2);
//     std::cout << request_t << "   " << response_t << "\n";
//     std::cout << corrected_age_value(&response);
// }

// int main(){
//     test_calculate_freshness();
//     return EXIT_SUCCESS;
// }
