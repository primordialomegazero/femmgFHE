#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Grover's (fixed diffusion)...\n\n";
    
    const double PI = 3.14159265358979323846;
    int N = 8;
    int iterations = static_cast<int>(std::floor(PI / 4.0 * std::sqrt(N)));  // 2
    
    bool all_passed = true;
    
    for (int target = 0; target < 8; target++) {
        // Classical simulation ng Grover's (tamang amplitude amplification)
        std::vector<double> amplitudes(N, 1.0 / std::sqrt(N));
        
        // Oracle: i-flip ang sign ng target
        // Diffusion: inversion about mean
        for (int iter = 0; iter < iterations; iter++) {
            // Oracle
            amplitudes[target] = -amplitudes[target];
            
            // Diffusion: 2*mean - amplitude
            double mean = 0.0;
            for (auto& amp : amplitudes) mean += amp;
            mean /= N;
            
            for (auto& amp : amplitudes) {
                amp = 2.0 * mean - amp;
            }
        }
        
        // I-measure: hanapin ang pinakamataas na probability
        int measured = 0;
        double max_prob = -1.0;
        for (int i = 0; i < N; i++) {
            double prob = amplitudes[i] * amplitudes[i];
            if (prob > max_prob) {
                max_prob = prob;
                measured = i;
            }
        }
        
        std::cout << "Target " << target << ": measured=" << measured 
                  << " prob=" << max_prob * 100 << "%\n";
        
        if (measured != target) {
            all_passed = false;
        }
    }
    
    if (all_passed) {
        std::cout << "\n✅ GROVER'S (CLASSICAL SIMULATION) PASSED!\n";
        std::cout << "Ito ang tamang amplitude amplification.\n";
        std::cout << "Para sa encrypted version, kailangan natin ng homomorphic mean at inversion.\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
