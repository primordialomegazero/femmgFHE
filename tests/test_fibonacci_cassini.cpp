// FIBONACCI CASSINI — Natural Computation Space
// Cassini: F(n-1)F(n+1) - F(n)² = (-1)^n
// Ito ay natural na ±1 oscillation — natural sign function
// Hindi kailangan ng external threshold

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIBONACCI CASSINI\n";
    std::cout << "  Natural ±1 Oscillation\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

    // Fibonacci at Lucas numbers
    std::cout << "FIBONACCI AT LUCAS NUMBERS:\n";
    std::cout << "===========================\n\n";
    
    long long F[15];
    F[0] = 0; F[1] = 1;
    for (int i = 2; i < 15; i++) F[i] = F[i-1] + F[i-2];
    
    long long L[15];
    L[0] = 2; L[1] = 1;
    for (int i = 2; i < 15; i++) L[i] = L[i-1] + L[i-2];
    
    std::cout << "  n  | F(n) | L(n)\n";
    std::cout << "  ---|------|-----\n";
    for (int n = 0; n < 12; n++) {
        std::cout << "  " << n << "   | " << F[n] << "    | " << L[n] << "\n";
    }
    std::cout << "\n";

    // Cassini identity test
    std::cout << "CASSINI IDENTITY TEST:\n";
    std::cout << "======================\n\n";
    
    for (int n = 1; n < 12; n++) {
        long long cassini = F[n-1] * F[n+1] - F[n] * F[n];
        int sign = (n % 2 == 0) ? 1 : -1;
        std::cout << "  F(" << (n-1) << ")F(" << (n+1) << ") - F(" << n << ")² = "
                  << cassini << " (expected " << sign << ")\n";
    }
    std::cout << "\n";

    // Natural sign function: (-1)^n
    std::cout << "NATURAL SIGN FUNCTION:\n";
    std::cout << "======================\n\n";
    std::cout << "  (-1)^n ay natural na ±1\n";
    std::cout << "  Ito ay level 0 — walang multiplication\n\n";

    // Binet formula: F(n) = (φ^n - ψ^n) / √5
    std::cout << "BINET FORMULA:\n";
    std::cout << "==============\n\n";
    std::cout << "  F(n) = (φ^n - ψ^n) / √5\n";
    std::cout << "  Ito ay nagbibigay ng natural na computation\n\n";

    // Ang susi: Fibonacci addition formula
    // F(m+n) = F(m)F(n+1) + F(m-1)F(n)
    std::cout << "FIBONACCI ADDITION:\n";
    std::cout << "===================\n\n";
    std::cout << "  F(m+n) = F(m)F(n+1) + F(m-1)F(n)\n";
    std::cout << "  Ito ay may multiplication — level cost\n\n";

    // Pero may natural na representation
    // Lucas: L(n) = φ^n + ψ^n
    std::cout << "LUCAS REPRESENTATION:\n";
    std::cout << "=====================\n\n";
    std::cout << "  L(n) = φ^n + ψ^n\n";
    std::cout << "  L(0)=2, L(1)=1, L(2)=3, L(3)=4\n";
    std::cout << "  Natural integers, bounded sa Fibonacci space\n\n";

    // Test: Lucas addition
    // L(m+n) = L(m)L(n) - (-1)^n L(m-n)
    std::cout << "LUCAS ADDITION:\n";
    std::cout << "===============\n\n";
    std::cout << "  L(m+n) = L(m)L(n) - (-1)^n L(m-n)\n";
    std::cout << "  May multiplication pa rin\n\n";

    // Hmm... ang Cassini ang susi
    // F(n-1)F(n+1) - F(n)² = (-1)^n
    // Kung n ay even → +1
    // Kung n ay odd → -1
    std::cout << "CASSINI AS NATURAL SIGN:\n";
    std::cout << "========================\n\n";
    std::cout << "  Kahit na may multiplication, ang output ay ±1\n";
    std::cout << "  Ito ay natural na threshold — walang external\n";
    std::cout << "  comparison na kailangan\n\n";

    return 0;
}
