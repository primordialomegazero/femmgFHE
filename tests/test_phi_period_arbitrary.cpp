// ARBITRARY PERIOD-N — CORRECTED DETECTION
// Ayusin ang period detection para sa Fibonacci mod k·φ²

#include <iostream>
#include <vector>
#include <cmath>
#include <map>

int main() {
    std::cout << "========================================\n";
    std::cout << "  ARBITRARY PERIOD-N SEARCH (FIXED)\n";
    std::cout << "  Fibonacci mod k·φ²\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    // Fibonacci numbers (mas malaki para sa longer periods)
    auto fib = [](int n) -> long long {
        long long a = 0, b = 1;
        for (int i = 0; i < n; i++) {
            long long temp = a + b;
            a = b;
            b = temp;
        }
        return a;
    };

    // Hanapin ang period para sa iba't ibang k (mod k·φ²)
    std::cout << "PERIOD SEARCH (mod k·φ²):\n";
    std::cout << "==========================\n\n";
    std::cout << "  k  | Period | States (unique values)\n";
    std::cout << "  ---|--------|------------------------\n";

    for (int k = 1; k <= 30; k++) {
        double modulus = k * phi_sq;
        std::vector<double> sequence;
        std::vector<double> unique_states;
        
        // Generate sequence
        for (int n = 0; n <= 200; n++) {
            double fib_scaled = fib(n) * phi_sq;
            double mod_val = std::fmod(fib_scaled, modulus);
            sequence.push_back(mod_val);
            
            // I-check kung unique
            bool is_unique = true;
            for (double s : unique_states) {
                if (std::abs(s - mod_val) < 0.001) {
                    is_unique = false;
                    break;
                }
            }
            if (is_unique) {
                unique_states.push_back(mod_val);
            }
        }
        
        // Hanapin ang period: shortest cycle na bumabalik sa simula
        int period = 0;
        for (int p = 1; p <= 100; p++) {
            bool is_period = true;
            for (int i = 0; i < 100; i++) {
                if (std::abs(sequence[i] - sequence[i + p]) > 0.001) {
                    is_period = false;
                    break;
                }
            }
            if (is_period) {
                period = p;
                break;
            }
        }
        
        std::cout << "  " << k << "  | Period-" << period 
                  << " | " << unique_states.size() << " unique states: ";
        for (size_t i = 0; i < std::min(unique_states.size(), (size_t)6); i++) {
            std::cout << unique_states[i] << " ";
        }
        if (unique_states.size() > 6) std::cout << "...";
        std::cout << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY FINDINGS:\n";
    std::cout << "  =============\n";
    std::cout << "  Period-N ay nag-iiba sa bawat k!\n";
    std::cout << "  Mas malaking k = mas mahabang period?\n";
    std::cout << "========================================\n";

    return 0;
}
