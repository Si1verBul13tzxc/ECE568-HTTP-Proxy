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
}  // namespace parser_method
