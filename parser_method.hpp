#include <memory>

#include "httpparser/httprequestparser.h"
#include "httpparser/httpresponseparser.h"
#include "httpparser/request.h"
#include "httpparser/response.h"
#include "my_exception.hpp"
namespace parser_method {
  void get_host_and_port(const httpparser::Request & req,
                         std::string & hostname,
                         std::string & port);
  std::unique_ptr<httpparser::Request> http_request_parse(std::vector<char> & buffer);
  std::unique_ptr<httpparser::Response> http_response_parse(std::vector<char> & buffer);
  std::string get_request_line(const httpparser::Request & req);
  std::string get_response_line(const httpparser::Response & resp);
}  // namespace parser_method
