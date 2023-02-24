#include <assert.h>
#include <poll.h>

#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Socket_connection.hpp"
#include "parser_method.hpp"
class Doubly_node {
 public:
  std::string key;
  std::unique_ptr<httpparser::Response> value;
  Doubly_node * prev;
  Doubly_node * next;
  Doubly_node(){};
  Doubly_node(std::string uri, std::unique_ptr<httpparser::Response> buffer) :
      key(uri), value(std::move(buffer)){};
};

class Cache {
 private:
  std::unordered_map<std::string, Doubly_node *> response_map;
  size_t size;
  size_t capacity;
  Doubly_node * head;
  Doubly_node * tail;

  void add_to_head(Doubly_node * response);
  void move_to_head(Doubly_node * response);
  void removeNode(Doubly_node * response);
  Doubly_node * remove_tail();

 public:
  Cache(size_t capacity) :
      size(0), capacity(capacity), head(new Doubly_node()), tail(new Doubly_node()) {
    head->next = tail;
    tail->prev = head;
  }

  void add_response(std::string uri, std::unique_ptr<httpparser::Response> buffer);
  httpparser::Response * get_response(std::string uri);
};
