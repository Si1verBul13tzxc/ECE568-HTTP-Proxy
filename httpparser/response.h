/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef HTTPPARSER_RESPONSE_H
#define HTTPPARSER_RESPONSE_H

#include <sstream>
#include <string>
#include <vector>

namespace httpparser {

  struct Response {
    Response() : versionMajor(0), versionMinor(0), keepAlive(false), statusCode(0) {}

    struct HeaderItem {
      std::string name;
      std::string value;
    };

    int versionMajor;
    int versionMinor;
    std::vector<HeaderItem> headers;
    std::vector<char> content;
    bool keepAlive;

    unsigned int statusCode;
    std::string status;

    std::string inspect() const {
      std::stringstream stream;
      stream << "HTTP/" << versionMajor << "." << versionMinor << " " << statusCode << " "
             << status << "\r\n";

      for (std::vector<Response::HeaderItem>::const_iterator it = headers.begin();
           it != headers.end();
           ++it) {
        stream << it->name << ": " << it->value << "\r\n";
      }
      stream << "\r\n";
      std::string data(content.begin(), content.end());
      stream << data;
      return stream.str();
    }
  };

}  // namespace httpparser

#endif  // HTTPPARSER_RESPONSE_H
