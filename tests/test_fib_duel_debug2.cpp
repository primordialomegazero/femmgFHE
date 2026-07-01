#include "../src/chaos/fibonacci_duel.h"
#include <iostream>
#include <iomanip>

using namespace fibonacci_duel;

int main() {
    FibonacciDuelEngine engine;
    
    std::cout << "=== DEEP DEBUG: 42 vs 43 ===" << std::endl;
    
    auto [ct42, h42] = engine.observe(42.0);
    auto [ct43, h43] = engine.observe(43.0);
    
    std::cout << "Final ct42: " << ct42 << std::endl;
    std::cout << "Final ct43: " << ct43 << std::endl;
    std::cout << "Final diff: " << (ct42 - ct43) << std::endl;
    
    std::cout << "\nLayer-by-layer comparison:" << std::endl;
    double running42 = 42.0;
    double running43 = 43.0;
    
    for (int i = 0; i < LAYERS; i++) {
        running42 += h42[i];
        running43 += h43[i];
        double layer_diff = running42 - running43;
        
        std::cout << "  Layer " << i << " (" << (i%2==0?"F":"R") << "): "
                  << "h42=" << std::setw(10) << h42[i]
                  << " h43=" << std::setw(10) << h43[i]
                  << " running42=" << std::setw(12) << running42
                  << " running43=" << std::setw(12) << running43
                  << " diff=" << layer_diff
                  << std::endl;
    }
    
    // Check: is the initial diff of 1.0 getting preserved?
    std::cout << "\nInitial diff was 1.0. Final diff is " << (ct42 - ct43) << std::endl;
    std::cout << "The chaos is PRESERVING the initial difference, not AMPLIFYING it!" << std::endl;
    
    return 0;
}
