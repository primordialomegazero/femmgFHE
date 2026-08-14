#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <vector>
#include <cmath>

// Grover's na may integer (fixed-point) amplitudes
// Para ma-encrypt natin ang amplitudes bilang integers

const int SCALE = 1000000;  // 6 decimal places

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Grover's na may Integer Amplitudes...\n\n";
    
    const double PI = 3.14159265358979323846;
    int N = 8;
    int iterations = 2;  // floor(pi/4 * sqrt(8))
    
    bool all_passed = true;
    
    for (int target = 0; target < 8; target++) {
        // Integer amplitudes
        std::vector<int> amplitudes(N);
        int initial = static_cast<int>(SCALE / std::sqrt(N));  // 353553
        
        for (int i = 0; i < N; i++) {
            amplitudes[i] = initial;
        }
        
        // Grover iterations na may integer arithmetic
        for (int iter = 0; iter < iterations; iter++) {
            // Oracle: i-flip ang sign ng target
            amplitudes[target] = -amplitudes[target];
            
            // Diffusion: 2*mean - amplitude
            // mean = sum / N
            int sum = 0;
            for (auto& amp : amplitudes) sum += amp;
            int mean = sum / N;
            
            for (auto& amp : amplitudes) {
                amp = 2 * mean - amp;
            }
        }
        
        // I-measure: hanapin ang pinakamataas na |amplitude|
        int measured = 0;
        int max_abs = -1;
        for (int i = 0; i < N; i++) {
            int abs_val = std::abs(amplitudes[i]);
            if (abs_val > max_abs) {
                max_abs = abs_val;
                measured = i;
            }
        }
        
        // Probability = (amplitude/SCALE)^2
        double prob = std::pow(amplitudes[measured] / static_cast<double>(SCALE), 2);
        
        std::cout << "Target " << target << ": measured=" << measured 
                  << " prob=" << prob * 100 << "%\n";
        
        if (measured != target) {
            all_passed = false;
            std::cout << "  amplitudes: ";
            for (auto& amp : amplitudes) std::cout << amp << " ";
            std::cout << "\n";
        }
    }
    
    if (all_passed) {
        std::cout << "\n✅ GROVER'S INTEGER PASSED! Lahat ng 8 targets found!\n";
        std::cout << "Ngayon kaya na nating i-encrypt ang amplitudes bilang integers.\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
