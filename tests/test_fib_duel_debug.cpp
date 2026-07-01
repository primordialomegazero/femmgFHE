#include "../src/chaos/fibonacci_duel.h"
#include <iostream>
#include <iomanip>

using namespace fibonacci_duel;

int main() {
    FibonacciDuelEngine engine;
    
    std::cout << "=== FIB DUEL DEBUG ===" << std::endl;
    
    // Test: observe 42 and 43, show each layer
    for (double val : {42.0, 43.0}) {
        std::cout << "\nValue: " << val << std::endl;
        auto [ct, hist] = engine.observe(val);
        
        std::cout << "  Final ct: " << ct << std::endl;
        std::cout << "  Layer deltas:" << std::endl;
        for (int i = 0; i < LAYERS; i++) {
            std::cout << "    Layer " << i << " (" << (i%2==0?"FWD":"REV") << "): " 
                      << std::setprecision(6) << hist[i] << std::endl;
        }
    }
    
    // Test: what if we make chaos stronger?
    std::cout << "\n=== MANUAL CHAOS AMPLIFICATION ===" << std::endl;
    double x = 42.0;
    for (int i = 0; i < 3; i++) {
        double floor = FLOORS[i];
        double contracted = x * PHI_INV + floor * (1.0 - PHI_INV);
        // Original chaos
        double chaos_orig = PHI * std::sin(contracted * PHI + i * PHI_INV);
        // Amplified chaos (×100)
        double chaos_amp = PHI * 100.0 * std::sin(contracted * PHI + i * PHI_INV);
        std::cout << "  Layer " << i << ": chaos_orig=" << chaos_orig 
                  << " chaos_amp=" << chaos_amp 
                  << " contracted=" << contracted << std::endl;
    }
    
    return 0;
}
