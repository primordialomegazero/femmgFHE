// SIMPLE FHE — φ encoding + minimal noise (walang pk)
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "SIMPLE FHE — MINIMAL NOISE\n";
    std::cout << "==========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    // Manual encryption: ct = (m·φ, 0) — walang pk, walang u, walang e
    auto ct0 = std::make_pair(
        []() { NTL::ZZ_pX p; NTL::SetCoeff(p, 0, NTL::to_ZZ_p(0)); return p; }(),
        []() { NTL::ZZ_pX p; NTL::SetCoeff(p, 0, NTL::to_ZZ_p(0)); return p; }()
    );
    
    auto ct1 = std::make_pair(
        [&fhe]() { NTL::ZZ_pX p; NTL::SetCoeff(p, 0, fhe.golden_plain_p); return p; }(),
        [&fhe]() { NTL::ZZ_pX p; NTL::SetCoeff(p, 0, NTL::to_ZZ_p(0)); return p; }()
    );
    
    // Test decrypt
    std::cout << "ct0 decrypts: " << fhe.decrypt(ct0) << " (exp 0)\n";
    std::cout << "ct1 decrypts: " << fhe.decrypt(ct1) << " (exp 1)\n\n";
    
    // Deep NAND chain
    auto current = ct1;
    std::cout << "Deep chain:\n";
    for (int i = 0; i <= 10; i++) {
        bool result = fhe.decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        std::cout << "  Depth " << i << ": " << result << " (exp " << expected << ") "
                  << (result == expected ? "✓" : "✗") << "\n";
        current = fhe.nand_gate(current, current);
    }
    
    return 0;
}
