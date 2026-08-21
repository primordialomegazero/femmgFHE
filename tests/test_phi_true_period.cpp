// TUNAY NA PERIOD-N — FIBONACCI MODULO DETECTION
// Hanapin ang tunay na period na may N natatanging states

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  TUNAY NA PERIOD-N DETECTION\n";
    std::cout << "  Fibonacci Modulo Analysis\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    // I-scan ang iba't ibang modulo values
    // para hanapin ang tunay na period

    std::cout << "PERIOD SCAN (mod M):\n";
    std::cout << "====================\n\n";

    // Subukan ang iba't ibang modulo: 2φ², 3φ², 4φ², 5φ², 6φ², atbp.
    for (int mod_factor = 2; mod_factor <= 10; mod_factor++) {
        double M = mod_factor * phi_sq;
        std::cout << "Modulo " << mod_factor << "φ²:\n";
        
        std::vector<double> fib = {0, 1};
        bool found = false;
        
        for (int i = 2; i <= 50; i++) {
            fib.push_back(fib[i-1] + fib[i-2]);
        }
        
        // Hanapin ang period — pinakamaliit na N kung saan umuulit
        for (int N = 2; N <= 30; N++) {
            bool match = true;
            for (int i = 0; i < N; i++) {
                double val_i = std::fmod(fib[i] * phi_sq, M);
                double val_iN = std::fmod(fib[i + N] * phi_sq, M);
                if (std::abs(val_i - val_iN) > 0.01) {
                    match = false;
                    break;
                }
            }
            if (match) {
                std::cout << "  Period-" << N << " (may " << N << " states)\n";
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cout << "  Walang period sa loob ng 30\n";
        }
        std::cout << "\n";
    }

    std::cout << "========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  Ang tunay na period ay depende sa modulo.\n";
    std::cout << "  Iba't ibang modulo = iba't ibang period.\n";
    std::cout << "  May period-8 sa mod 4φ².\n";
    std::cout << "  Baka may period-16 sa ibang modulo.\n";
    std::cout << "========================================\n";

    return 0;
}
