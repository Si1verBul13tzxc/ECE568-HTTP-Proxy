#include "cache.hpp"

void Cache::addToHead(Doubly_node * response){
    response->next = Cache::head;
    Cache::head->prev = response;
    Cache::head = response;
    Cache::head->prev = NULL;
    Cache::size++;
}

void Cache::moveToHead(Doubly_node * response){
    if(response == Cache::head){
        return;
    }
    remove(response);
    addToHead(response);
}

void Cache::remove(Doubly_node * response){
    response->prev->next = response->next;
    response->next->prev = response->prev;
    Cache::size --;
}

void Cache::add_response(std::string uri, std::vector<char> buffer){
    Doubly_node * response = new Doubly_node(uri, buffer);
    addToHead(response);
}

Doubly_node * Cache::get_response(std::string uri){
    std::unordered_map<std::string, Doubly_node*>::const_iterator it = Cache::response_map->find(uri);
    if(it == Cache::response_map->end()){
        //not find in the cache
        return NULL;
    }else{
        moveToHead(it->second);
        return it->second;
    }
}
