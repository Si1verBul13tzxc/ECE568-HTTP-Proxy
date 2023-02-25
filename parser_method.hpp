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
  std::string request_get_header_value(const httpparser::Request & req,
                                       const std::string header_name);
  std::string response_get_header_value(const httpparser::Response & resp,
                                        const std::string header_name);
  void update_response(httpparser::Response & resp, std::string name, std::string value);
}  // namespace parser_method
