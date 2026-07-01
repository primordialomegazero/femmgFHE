#include "../src/chaos/eight_demon_gates.h"
#include <iostream>
#include <iomanip>

using namespace eight_demon_gates;

int main() {
    std::cout << std::setprecision(15);
    std::cout << "=== 8 DEMON GATES DEBUG ===" << std::endl;
    
    EightDemonGatesEngine engine;
    
    // Test observe
    auto [ct, hist] = engine.observe(42.0);
    std::cout << "After observe(42): " << ct << std::endl;
    
    // Show first few history values
    std::cout << "\nFirst 5 history values:" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "  hist[" << i << "] = " << hist[i] << std::endl;
    }
    
    // Show last 5 history values
    std::cout << "\nLast 5 history values:" << std::endl;
    for (int i = 43; i < 48; i++) {
        std::cout << "  hist[" << i << "] = " << hist[i] << std::endl;
    }
    
    // Test unobserve
    std::cout << "\n=== UNOBSERVE ===" << std::endl;
    double x = ct;
    std::cout << "Starting x: " << x << std::endl;
    
    // Reverse one layer at a time, show last 3
    for (int i = 47; i >= 45; i--) {
        x -= hist[i];
        std::cout << "After reversing layer " << i << ": x = " << x << std::endl;
    }
    
    // Full reversal
    double pt = engine.unobserve(ct, hist);
    std::cout << "\nFinal unobserve result: " << pt << " (expected 42)" << std::endl;
    std::cout << "Error: " << (pt - 42.0) << std::endl;
    
    return 0;
}
