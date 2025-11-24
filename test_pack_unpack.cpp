#include "jansson-cpp/include/jansson.hpp"
#include <iostream>

int main() {
    // Test basic packing functionality
    json_t* obj = json_pack("{s:i}", "key", 42);
    if (obj) {
        std::cout << "Pack test passed" << std::endl;
        json_decref(obj);
    } else {
        std::cout << "Pack test failed" << std::endl;
    }
    
    return 0;
}
