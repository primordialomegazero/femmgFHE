#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

constexpr long Q = 536870909;

int main() {
    std::cout << "FIBONACCI RELINEARIZATION TEST\n\n";
    
    NTL::ZZ_p::init(NTL::ZZ(Q));
    
    // IDEA: Kung ang s(x) ay may Fibonacci coefficients,
    // ang s² ay may natural na reduction
    
    std::cout << "Fibonacci sequence: 1, 1, 2, 3, 5, 8, 13, 21, ...\n";
    std::cout << "F(n) = F(n-1) + F(n-2)\n\n";
    
    std::cout << "Golden ratio connection:\n";
    std::cout << "  F(n+1)/F(n) → φ as n → ∞\n\n";
    
    std::cout << "Kung ang secret key s ay may coefficients F(n):\n";
    std::cout << "  s(x) = Σ F(i) · x^i\n\n";
    
    // Test: Fibonacci polynomial
    NTL::ZZ_pX s;
    long f0 = 0, f1 = 1;
    for (int i = 0; i < 10; i++) {
        NTL::SetCoeff(s, i, f1);
        long f2 = f0 + f1;
        f0 = f1;
        f1 = f2;
    }
    
    std::cout << "s(x) coefficients (first 10): ";
    f0 = 0; f1 = 1;
    for (int i = 0; i < 10; i++) {
        std::cout << f1 << " ";
        long f2 = f0 + f1;
        f0 = f1;
        f1 = f2;
    }
    std::cout << "\n\n";
    
    // Compute s²
    NTL::ZZ_pX s_sq = s * s;
    
    std::cout << "s²(x) coefficients (first 10):\n";
    for (int i = 0; i < 10; i++) {
        std::cout << "  [" << i << "] = " << NTL::coeff(s_sq, i) << "\n";
    }
    
    std::cout << "\nObservation:\n";
    std::cout << "Ang s² ay may pattern na pwedeng ma-predict\n";
    std::cout << "Kung ang pattern ay known, baka automatic ang reduction\n";
    
    return 0;
}
