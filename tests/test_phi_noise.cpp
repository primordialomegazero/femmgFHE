#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "φ-NOISE OSCILLATION TEST\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    // I-check kung ang noise ay nag-o-oscillate
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "Starting ciphertext ct1\n";
    
    // Manual noise extraction
    NTL::ZZ_pX noise = ct1.first + ct1.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v0 = NTL::rep(NTL::coeff(noise, 0));
    std::cout << "Initial v: " << v0 << "\n";
    
    // Apply NAND chain
    auto current = ct1;
    for (int i = 1; i <= 10; i++) {
        current = fhe.nand_gate(current, current);
        NTL::ZZ_pX n = current.first + current.second * fhe.s;
        fhe.reduce_mod(n);
        NTL::ZZ v = NTL::rep(NTL::coeff(n, 0));
        
        std::cout << "After NAND[" << i << "]: v=" << v 
                  << " decrypt=" << fhe.decrypt(current)
                  << " expected=" << (i % 2 == 0) << "\n";
    }
    
    return 0;
}
