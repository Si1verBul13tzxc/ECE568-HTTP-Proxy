#include "httpparser/httprequestparser.h"
#include "httpparser/request.h"
#include "iostream"
int main() {
  char text[] = "GET /uri.cgi HTTP/1.1\r\nUser-Agent: Mozilla/5.0\r\nAccept: "
                "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
                "*;q=0.8\r\nHost: 127.0.0.1\r\n\r\n\0\0\0\asdfasfasf";
  httpparser::Request request;
  httpparser::HttpRequestParser parser;
  httpparser::HttpRequestParser::ParseResult res =
      parser.parse(request, text, text + strlen(text));

  if (res == httpparser::HttpRequestParser::ParsingCompleted) {
    std::cout << request.inspect() << std::endl;
    std::cout << request.method << std::endl;
    return EXIT_SUCCESS;
  }
  else {
    std::cerr << "Parsing failed" << std::endl;
    return EXIT_FAILURE;
  }
}
