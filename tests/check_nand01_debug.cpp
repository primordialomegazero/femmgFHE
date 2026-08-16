#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("18446744073709551611");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    auto nand01 = fhe.nand_gate(ct0, ct1);
    
    NTL::ZZ_pX noise = nand01.first + nand01.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
    
    std::cout << "NAND(0,1) v = " << v << "\n";
    std::cout << "golden_plain = " << fhe.golden_plain << "\n";
    std::cout << "Q/2 = " << Q/2 << "\n";
    std::cout << "dist_0 = " << ((v < Q/2) ? v : Q-v) << "\n";
    NTL::ZZ diff = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
    std::cout << "dist_golden = " << ((diff < Q/2) ? diff : Q-diff) << "\n";
    std::cout << "decrypt = " << fhe.decrypt(nand01) << "\n";
    
    // I-check ang product values
    NTL::ZZ_pX t0 = ct0.first * ct1.first;
    NTL::ZZ_pX t1 = ct0.first * ct1.second + ct0.second * ct1.first;
    NTL::ZZ_pX t2 = ct0.second * ct1.second;
    fhe.reduce_mod(t0); fhe.reduce_mod(t1); fhe.reduce_mod(t2);
    
    std::cout << "\nt0 coeff 0: " << NTL::coeff(t0, 0) << "\n";
    std::cout << "t1 coeff 0: " << NTL::coeff(t1, 0) << "\n";
    std::cout << "t2 coeff 0: " << NTL::coeff(t2, 0) << "\n";
    
    return 0;
}
