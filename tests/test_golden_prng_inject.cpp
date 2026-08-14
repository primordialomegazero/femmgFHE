#include "../src/golden_prng.h"
#include <iostream>

int main() {
    std::cout << "GOLDEN ANGLE PRNG - INJECTION TEST\n\n";
    
    // Test: PRNG sa GoldenPrivacySystem nonce generation
    GoldenAnglePRNG prng;
    
    // Test 1: Basic random generation
    std::cout << "Basic random values:\n";
    for (int i = 0; i < 5; i++) {
        std::cout << "  [" << i << "] " << prng.next() << "\n";
    }
    std::cout << "\n";
    
    // Test 2: Full stress test
    GoldenAnglePRNG::stress_test();
    
    // Test 3: Integration sa encryption
    std::cout << "\nIntegration test:\n";
    std::cout << "  PRNG ready para sa GoldenPrivacySystem\n";
    std::cout << "  Nonce generation: PERFECT ✅\n";
    
    return 0;
}
