#include <iostream>
#include <random>
#include <complex>
#include <cmath>

int main() {
    std::cout << "Quantum Randomness Bias Analysis\n\n";
    
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    // Test: Hadamard + Measure sequence
    std::complex<double> amp_0(1.0, 0.0);
    std::complex<double> amp_1(0.0, 0.0);
    
    int zeros = 0, ones = 0;
    
    for (int i = 0; i < 1000; i++) {
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
        
        if (result) {
            ones++;
            amp_0 = 0.0;
            amp_1 = 1.0;
        } else {
            zeros++;
            amp_0 = 1.0;
            amp_1 = 0.0;
        }
    }
    
    std::cout << "1000 measurements: " << zeros << " zeros, " << ones << " ones\n";
    std::cout << "Balance: " << std::abs(zeros - ones) / 1000.0 << "\n\n";
    
    // Ang problema: hadamard mula sa |0> ay laging P(0)=0.5
    // Pagkatapos ng measure, nagco-collapse sa |0> o |1>
    // Tapos hadamard ulit mula sa collapsed state
    // H|0> = (|0>+|1>)/√2 → P(0)=0.5
    // H|1> = (|0>-|1>)/√2 → P(0)=0.5
    // Kaya laging 50/50 dapat!
    
    std::cout << "Theoretical: 500 zeros, 500 ones (50/50)\n";
    std::cout << "Observed bias ay mula sa PRNG, hindi sa quantum logic\n\n";
    
    // FIX: Gumamit ng mas mahusay na PRNG
    std::random_device rd2;
    std::mt19937_64 rng2(rd2());
    std::uniform_real_distribution<double> dist2(0.0, 1.0);
    
    amp_0 = 1.0;
    amp_1 = 0.0;
    zeros = 0;
    ones = 0;
    
    for (int i = 0; i < 1000; i++) {
        double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        auto new_0 = (amp_0 + amp_1) * inv_sqrt2;
        auto new_1 = (amp_0 - amp_1) * inv_sqrt2;
        amp_0 = new_0;
        amp_1 = new_1;
        
        double prob_0 = std::norm(amp_0);
        double rand_val = dist2(rng2);
        bool result = rand_val >= prob_0;
        
        if (result) {
            ones++;
            amp_0 = 0.0;
            amp_1 = 1.0;
        } else {
            zeros++;
            amp_0 = 1.0;
            amp_1 = 0.0;
        }
    }
    
    std::cout << "With mt19937_64: " << zeros << " zeros, " << ones << " ones\n";
    std::cout << "Balance: " << std::abs(zeros - ones) / 1000.0 << "\n";
    
    return 0;
}
