// #include <iostream>
// #include "time_format.hpp"
// #include "httpparser/response.h"
// #include "httpparser/httpresponseparser.h"
// #include "parser_method.hpp"

// long test_format() {
//   std::string time = "Sun, 26 Feb 2023 17:00:00 GMT";
//   time_t transfer = time_format::to_tm_format(time);
//   return transfer;
// }

// void test_calculate_freshness(){
//     const char text[] =
//             "HTTP/1.1 200 OK\r\n"
//             "Server: nginx/1.2.1\r\n"
//             "Content-Type: text/html\r\n"
//             "Content-Length: 8\r\n"
//             "Connection: keep-alive\r\n"
//             "\r\n"
//             "<html />";

//     httpparser::Response response;
//     httpparser::HttpResponseParser parser;
//     httpparser::HttpResponseParser::ParseResult res = parser.parse(response, text, text + sizeof(text));
//     time_t request_time = time(0);
//     time_t response_time = time(0);
//     std::cout << request_time << "   " << response_time << "\n";
//     httpparser::Response::HeaderItem req_item = {"request_time", std::to_string(request_time)}; 
//     httpparser::Response::HeaderItem resp_item = {"response_time", std::to_string(response_time)}; 
//     response.headers.push_back(req_item);
//     response.headers.push_back(resp_item);
//     std::string response_time_str = parser_method::response_get_header_value(response, "response_time");
//     std::string request_time_str = parser_method::response_get_header_value(response, "request_time");
//     std::string::size_type sz1, sz2, sz3;
//     time_t request_t = std::stol(request_time_str, &sz1);
//     time_t response_t = std::stol(response_time_str, &sz2);
//     std::cout << request_t << "   " << response_t << "\n";
// }

// int main(){
//     std::cout << test_format();
//     return EXIT_SUCCESS;
// }
