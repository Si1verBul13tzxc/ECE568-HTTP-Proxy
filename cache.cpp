#include "cache.hpp"

void Cache::add_to_head(Doubly_node * response){
    response->next = Cache::head->next;
    Cache::head->next = response;
    response->prev = Cache::head;
    response->next->prev = response;
    Cache::size++;
    response_map.insert(std::make_pair(response->key, response));
    if(Cache::size > capacity){
        remove_tail();
    }
}

void Cache::move_to_head(Doubly_node * response){
    if(response == Cache::head){
        return;
    }
    remove(response);
    add_to_head(response);
}

void Cache::remove(Doubly_node * response){
    if(response == NULL){
        std::cerr<<"Cannot move null\n";
    }
    response->prev->next = response->next;
    response->next->prev = response->prev;
    Cache::size --;
}

void Cache::remove_tail(){
    remove(tail->prev);
}

void Cache::add_response(std::string uri, std::unique_ptr<httpparser::Response> buffer){
    Doubly_node * response = new Doubly_node(uri, std::move(buffer));
    add_to_head(response);
}

std::vector<char> * Cache::get_response(std::string uri){
    std::unordered_map<std::string, Doubly_node*>::const_iterator it = Cache::response_map.find(uri);
    std::cout << "The uri: "<< uri << std::endl;
    if(it == Cache::response_map.end()){
        std::cout << "Cannot find in cache\n";
        std::cout << "get request" << std::endl;
        //not find in the cache
        return NULL;
    }else{
        move_to_head(it->second);
        std::string response_str = (it->second->value)->inspect();
        std::vector<char> * response_buffer = new std::vector<char>();
        std::copy(response_str.begin(), response_str.end(), std::back_inserter(* response_buffer));
        return response_buffer;
    }
}

// void Cache::print_cache(){
//     auto it = Cache::response_map.begin();
//     while(it != Cache::response_map.end()){
//         std::string = httpparser::HttpResponseParser(it->second)
//     }
    
// }
