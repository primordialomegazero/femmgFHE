// EMERGENT PERIOD-N — MULA SA φ-DOMAIN
// Hanapin ang natural na period-N gamit ang Fibonacci

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  EMERGENT PERIOD-N SA φ-DOMAIN\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;

    // Fibonacci sequence
    std::vector<double> fib = {0, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    std::cout << "FIBONACCI NUMBERS:\n";
    for (int i = 0; i <= 15; i++) {
        std::cout << "  F(" << i << ") = " << fib[i] << "\n";
    }
    std::cout << "\n";

    // Period detection: φ^n mod 1
    std::cout << "φ-PERIODS (mod 1):\n";
    std::cout << "===================\n\n";

    for (int n = 1; n <= 20; n++) {
        double phi_n = std::pow(phi, n);
        double mod1 = phi_n - std::floor(phi_n);
        std::cout << "  φ^" << n << " mod 1 = " << mod1 << "\n";
    }
    std::cout << "\n";

    // Hanapin ang period-N kung saan φ^N ≈ integer + φ^0
    std::cout << "PERIOD-N SEARCH (φ^N ≈ integer + φ^M):\n";
    std::cout << "=========================================\n\n";

    for (int n = 1; n <= 20; n++) {
        double phi_n = std::pow(phi, n);
        double frac_part = phi_n - std::floor(phi_n);
        
        // I-check kung ang fractional part ay malapit sa φ^0 o φ^1
        if (std::abs(frac_part - 1.0/phi) < 0.01) {
            std::cout << "  PERIOD FOUND: φ^" << n << " = " 
                      << std::floor(phi_n) << " + ψ (ψ = 1/φ)\n";
        }
        if (std::abs(frac_part - 1.0) < 0.01) {
            std::cout << "  PERIOD FOUND: φ^" << n << " ≈ " 
                      << std::floor(phi_n) + 1 << "\n";
        }
    }

    std::cout << "\n";

    // Fibonacci period-N: F(N) mod M
    std::cout << "FIBONACCI PERIOD-N (mod 4φ²):\n";
    std::cout << "================================\n\n";

    const double four_phi_sq = 4 * phi * phi;
    for (int i = 0; i <= 20; i++) {
        double fib_mod = std::fmod(fib[i] * phi * phi, four_phi_sq);
        std::cout << "  F(" << i << ")·φ² mod 4φ² = " << fib_mod << "\n";
    }

    std::cout << "\n";

    // Ang pinakamahalagang obserbasyon:
    // φ^n = F(n)·φ + F(n-1)
    // Ito ay nagbibigay ng natural na period-N:
    // φ^(n+m) = φ^n · φ^m
    // = (F(n)φ + F(n-1))·(F(m)φ + F(m-1))
    // = F(n)F(m)φ² + F(n)F(m-1)φ + F(n-1)F(m)φ + F(n-1)F(m-1)
    // = F(n)F(m)(φ+1) + (F(n)F(m-1) + F(n-1)F(m))φ + F(n-1)F(m-1)

    std::cout << "EMERGENT PERIOD-N:\n";
    std::cout << "==================\n\n";
    std::cout << "  φ^n = F(n)·φ + F(n-1)\n";
    std::cout << "  Ito ay nagbibigay ng:\n";
    std::cout << "    φ^2 = φ + 1 (period-2)\n";
    std::cout << "    φ^3 = 2φ + 1 (period-3)\n";
    std::cout << "    φ^4 = 3φ + 2 (period-4)\n";
    std::cout << "    φ^5 = 5φ + 3 (period-5)\n";
    std::cout << "    φ^6 = 8φ + 5 (period-6)\n";
    std::cout << "    φ^N = F(N)φ + F(N-1) (period-N)\n\n";

    std::cout << "  Kaya ang φ ay natural na period-N generator!\n";
    std::cout << "  Ang bawat Fibonacci number ay nagbibigay ng bagong period.\n";

    return 0;
}
