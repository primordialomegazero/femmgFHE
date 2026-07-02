#include "../src/chaos/eight_demon_gates.h"
#include <iostream>
#include <chrono>

using namespace eight_demon_gates;

int main() {
    EightDemonGatesEngine engine;
    engine.set_maximum_mode();
    
    const int TOTAL = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for(int i=0; i<TOTAL; i++) {
        auto result = engine.observe(42.0, i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    std::cout << TOTAL << " observes in " << us << " µs = " 
              << (TOTAL * 1000000.0 / us) << " TPS" << std::endl;
    
    return 0;
}
