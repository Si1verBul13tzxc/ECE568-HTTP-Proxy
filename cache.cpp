#include "cache.hpp"

void Cache::add_to_head(Doubly_node * response) {
  response->next = head->next;
  head->next = response;
  response->prev = head;
  response->next->prev = response;
}

void Cache::move_to_head(Doubly_node * response) {
  removeNode(response);
  add_to_head(response);
}

void Cache::removeNode(Doubly_node * response) {
  response->prev->next = response->next;
  response->next->prev = response->prev;
}

Doubly_node * Cache::remove_tail() {
  Doubly_node * to_remove = tail->prev;
  removeNode(tail->prev);
  return to_remove;
}

void Cache::add_response(std::string uri, std::unique_ptr<httpparser::Response> buffer) {
  if (response_map.count(uri) == 0) {
    Doubly_node * response_node = new Doubly_node(
        uri, std::move(buffer));  //could fail here, but object state unchanged
    response_map[uri] = response_node;
    add_to_head(response_node);
    size++;
    if (size > capacity) {  //cache is full, need to drop least recent used response
      Doubly_node * to_remove = remove_tail();
      response_map.erase(to_remove->key);
      delete to_remove;  //unique pointer responsible to delete the repsonse object allocate on heap
      size--;
    }
  }
  else {  //key already exists, update
    Doubly_node * node = response_map[uri];
    node->value = std::move(buffer);  //move ownership
    move_to_head(node);               //update to be most recent used
  }
}

httpparser::Response * Cache::get_response(std::string uri) {
  auto it = response_map.find(uri);
  if (it == response_map.end()) {
    return NULL;
  }
  else {  //exists
    Doubly_node * response_node = it->second;
    move_to_head(response_node);
    return response_node->value.get();
  }
}
