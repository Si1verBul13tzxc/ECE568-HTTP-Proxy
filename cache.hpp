#include <assert.h>
#include <poll.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <unordered_map>
#include <list>

#include "Socket_connection.hpp"
#include "parser_method.hpp"
class Doubly_node{
    public:
        std::string key;
        std::vector<char> value;
        Doubly_node *prev;
        Doubly_node *next;
        Doubly_node();
        Doubly_node(std::string uri, std::vector<char> buffer) : key(uri), value(buffer){};
};

class Cache{
private:
    std::unordered_map<std::string, Doubly_node *> response_map;
    size_t size;
    Doubly_node *head;
    Doubly_node *tail;

    void addToHead(Doubly_node * response);
    void moveToHead(Doubly_node * response);
    void remove(Doubly_node * response);

public:
    Cache() : size(0), 
              head(new Doubly_node()),
              tail(new Doubly_node()){
                head->next = tail;
                tail->prev = head;
              }
    void add_response(std::string uri, std::vector<char> buffer);
    Doubly_node * get_response(std::string uri);


};
