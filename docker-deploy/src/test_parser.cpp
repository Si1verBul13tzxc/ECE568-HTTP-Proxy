// #include "httpparser/httprequestparser.h"
// #include "httpparser/request.h"
// #include "iostream"
// int test_parse() {
//   char text[] =
//       "POST /uri.cgi HTTP/1.1\r\n"
//       "From: user@example.com\r\n"
//       "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:18.0) Gecko/20100101 "
//       "Firefox/18.0\r\n"
//       "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
//       "Accept-Language: en-US,en;q=0.5\r\n"
//       "Accept-Encoding: gzip, deflate\r\n"
//       "Content-Type: application/x-www-form-urlencoded\r\n"
//       "Content-Length: 31\r\n"
//       "Host: 127.0.0.1\r\n"
//       "\r\n"
//       "arg1=test;arg1=%20%21;arg3=test";
//   httpparser::Request request;
//   httpparser::HttpRequestParser parser;
//   httpparser::HttpRequestParser::ParseResult res =
//       parser.parse(request, text, text + strlen(text));

//   if (res == httpparser::HttpRequestParser::ParsingCompleted) {
//     std::cout << request.inspect() << std::endl;
//     std::cout << request.method << std::endl;
//     std::cout << request.headers[7].name << std::endl;
//     return EXIT_SUCCESS;
//   }
//   else {
//     std::cerr << "Parsing failed" << std::endl;
//     return EXIT_FAILURE;
//   }
// }
