#include "httpparser/httprequestparser.h"
#include "httpparser/request.h"
#include "iostream"
int main() {
  char text[] = "CONNECT server.example.com:422 HTTP/1.1\r\n"
                "Proxy-Authorization: basic aGVsbG86d29ybGQ=\r\n\r\n";
  httpparser::Request request;
  httpparser::HttpRequestParser parser;
  httpparser::HttpRequestParser::ParseResult res =
      parser.parse(request, text, text + strlen(text));

  if (res == httpparser::HttpRequestParser::ParsingCompleted) {
    std::cout << request.inspect() << std::endl;
    std::cout << request.method << std::endl;
    std::cout << request.uri.substr(request.uri.find(":") + 1) << std::endl;
    std::cout << request.uri.substr(0, request.uri.find(":")) << std::endl;
    return EXIT_SUCCESS;
  }
  else {
    std::cerr << "Parsing failed" << std::endl;
    return EXIT_FAILURE;
  }
}
