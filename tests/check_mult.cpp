#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    
    // I-check ang ct0 polynomials
    std::cout << "ct0.c0 coeff 0: " << NTL::coeff(ct0.first, 0) << "\n";
    std::cout << "ct0.c1 coeff 0: " << NTL::coeff(ct0.second, 0) << "\n";
    
    // I-check ang decryption
    NTL::ZZ_pX noise = ct0.first + ct0.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v0 = NTL::rep(NTL::coeff(noise, 0));
    std::cout << "ct0 decrypt v: " << v0 << "\n\n";
    
    // Manual multiplication
    NTL::ZZ_pX t0 = ct0.first * ct0.first;
    NTL::ZZ_pX t1 = ct0.first * ct0.second + ct0.second * ct0.first;
    NTL::ZZ_pX t2 = ct0.second * ct0.second;
    fhe.reduce_mod(t0); fhe.reduce_mod(t1); fhe.reduce_mod(t2);
    
    std::cout << "t0 coeff 0: " << NTL::coeff(t0, 0) << "\n";
    std::cout << "t1 coeff 0: " << NTL::coeff(t1, 0) << "\n";
    std::cout << "t2 coeff 0: " << NTL::coeff(t2, 0) << "\n";
    
    return 0;
}
