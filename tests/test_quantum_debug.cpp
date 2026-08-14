#include <iostream>
#include <complex>
#include <cmath>
#include <random>

constexpr std::complex<double> I(0.0, 1.0);

int main() {
    std::cout << "Quantum Randomness Debug\n\n";
    
    // Test: Hadamard + Measure sequence
    std::complex<double> amp_0(1.0, 0.0);
    std::complex<double> amp_1(0.0, 0.0);
    
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    std::cout << "Hadamard + Measure (10 iterations):\n";
    
    for (int i = 0; i < 10; i++) {
        // Hadamard
        double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        auto new_0 = (amp_0 + amp_1) * inv_sqrt2;
        auto new_1 = (amp_0 - amp_1) * inv_sqrt2;
        amp_0 = new_0;
        amp_1 = new_1;
        
        // Measure
        double prob_0 = std::norm(amp_0);
        double rand_val = dist(rng);
        
        bool result = rand_val >= prob_0;
        
        std::cout << "  Iter " << i << ": P(0)=" << prob_0 
                  << " rand=" << rand_val 
                  << " result=" << result << "\n";
        
        // Collapse
        if (result) {
            amp_0 = 0.0;
            amp_1 = 1.0;
        } else {
            amp_0 = 1.0;
            amp_1 = 0.0;
        }
    }
    
    std::cout << "\n";
    std::cout << "Ang problema: mt19937(42) ay deterministic\n";
    std::cout << "Kaya ang sequence ng rand_val ay pareho every run\n";
    std::cout << "Kailangan natin ng true randomness source\n\n";
    
    // Test: gamitin ang random_device para sa true randomness
    std::random_device rd;
    std::mt19937 rng2(rd());
    std::uniform_real_distribution<double> dist2(0.0, 1.0);
    
    std::cout << "With random_device seed:\n";
    amp_0 = 1.0;
    amp_1 = 0.0;
    
    for (int i = 0; i < 10; i++) {
        double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        auto new_0 = (amp_0 + amp_1) * inv_sqrt2;
        auto new_1 = (amp_0 - amp_1) * inv_sqrt2;
        amp_0 = new_0;
        amp_1 = new_1;
        
        double prob_0 = std::norm(amp_0);
        double rand_val = dist2(rng2);
        
        bool result = rand_val >= prob_0;
        
        std::cout << "  Iter " << i << ": P(0)=" << prob_0 
                  << " rand=" << rand_val 
                  << " result=" << result << "\n";
        
        if (result) {
            amp_0 = 0.0;
            amp_1 = 1.0;
        } else {
            amp_0 = 1.0;
            amp_1 = 0.0;
        }
    }
    
    std::cout << "\n";
    std::cout << "✅ FIX: Gumamit ng random_device para sa true randomness\n";
    
    return 0;
}
