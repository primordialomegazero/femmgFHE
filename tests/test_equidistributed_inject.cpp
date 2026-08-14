#include "../src/golden_equidistributed.h"
#include <iostream>

int main() {
    std::cout << "GOLDEN EQUIDISTRIBUTED NOISE - INJECTION TEST\n\n";
    
    // Test 1: Basic noise generation
    GoldenEquidistributedNoise noise;
    
    std::cout << "Basic noise values:\n";
    for (int i = 0; i < 5; i++) {
        std::cout << "  [" << i << "] " << noise.next_noise() << "\n";
    }
    std::cout << "\n";
    
    // Test 2: Encryption noise
    GoldenEquidistributedNoise enc_noise;
    std::cout << "Encryption noise (max=1000):\n";
    for (int i = 0; i < 5; i++) {
        std::cout << "  [" << i << "] " << enc_noise.next_encryption_noise(1000) << "\n";
    }
    std::cout << "\n";
    
    // Test 3: Stress test
    GoldenEquidistributedNoise::stress_test();
    
    return 0;
}
