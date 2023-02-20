#include <exception>
class my_exception : public std::exception {
  const char * message;

 public:
  my_exception(const char * _message) : message(_message) {}
  virtual const char * what() const throw() { return this->message; }
};
