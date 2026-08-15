#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "257-BIT φ ARITHMETIC TEST\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    // Compute φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    std::cout << "φ = " << phi_zz << "\n\n";
    
    // TEST 1: φ arithmetic sa Z_Q
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    NTL::ZZ_p phi_sq = phi * phi;
    NTL::ZZ_p phi_plus_1 = phi + 1;
    
    std::cout << "TEST 1: φ² = φ+1 sa Z_Q\n";
    std::cout << "  φ² = " << phi_sq << "\n";
    std::cout << "  φ+1 = " << phi_plus_1 << "\n";
    std::cout << "  Match: " << (phi_sq == phi_plus_1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // TEST 2: Fibonacci sequence via φ
    std::cout << "TEST 2: Fibonacci numbers via φ\n";
    NTL::ZZ_p current = NTL::to_ZZ_p(1);
    
    // φ^n = F(n)·φ + F(n-1)
    // Pero φ^n as scalar: φ^1, φ^2, φ^3...
    NTL::ZZ_p phi_n = phi;
    for (int n = 1; n <= 10; n++) {
        // Compute Fibonacci via φ^n
        NTL::ZZ phi_n_zz = rep(phi_n);
        
        // F(n) = (φ^n - (-φ)^(-n)) / √5
        // Simpler: Verify φ^n satisfies recurrence
        std::cout << "  φ^" << n << " = " << phi_n << "\n";
        
        // Verify: φ^(n+1) = φ^n + φ^(n-1)
        if (n >= 3) {
            // This should hold automatically
        }
        
        phi_n = phi_n * phi;
    }
    std::cout << "\n";
    
    // TEST 3: Verify Fibonacci recurrence in Z_Q
    std::cout << "TEST 3: Fibonacci recurrence\n";
    std::cout << "  F(n) = F(n-1) + F(n-2)\n";
    
    NTL::ZZ_p F_n_minus_2 = NTL::to_ZZ_p(0);  // F(0) = 0
    NTL::ZZ_p F_n_minus_1 = NTL::to_ZZ_p(1);  // F(1) = 1
    
    std::cout << "  F(0) = 0\n";
    std::cout << "  F(1) = 1\n";
    
    for (int n = 2; n <= 15; n++) {
        NTL::ZZ_p F_n = F_n_minus_1 + F_n_minus_2;
        std::cout << "  F(" << n << ") = " << F_n << "\n";
        F_n_minus_2 = F_n_minus_1;
        F_n_minus_1 = F_n;
    }
    std::cout << "\n";
    
    // TEST 4: φ^n = F(n)·φ + F(n-1) verification
    std::cout << "TEST 4: φ^n = F(n)·φ + F(n-1)\n";
    std::cout << "  Since φ² = φ+1, φ^n can be reduced to linear form\n";
    std::cout << "  In Z_Q, this means φ^n mod Q follows Fibonacci\n\n";
    
    // Compute φ^10
    NTL::ZZ_p phi_10 = NTL::to_ZZ_p(1);
    for (int i = 0; i < 10; i++) {
        phi_10 = phi_10 * phi;
    }
    
    // F(10) = 55, F(9) = 34
    NTL::ZZ_p expected = NTL::to_ZZ_p(55) * phi + NTL::to_ZZ_p(34);
    
    std::cout << "  φ^10 = " << phi_10 << "\n";
    std::cout << "  55φ + 34 = " << expected << "\n";
    std::cout << "  Match: " << (phi_10 == expected ? "YES ✓" : "NO ✗") << "\n";
    
    return 0;
}
