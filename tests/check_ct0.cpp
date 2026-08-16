#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    
    NTL::ZZ_pX noise = ct0.first + ct0.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
    
    std::cout << "ct0 v = " << v << "\n";
    std::cout << "dist_0 = " << ((v < Q/2) ? v : Q - v) << "\n";
    NTL::ZZ diff = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
    std::cout << "dist_golden = " << ((diff < Q/2) ? diff : Q - diff) << "\n";
    std::cout << "decrypt = " << fhe.decrypt(ct0) << " (exp 0)\n";
    
    return 0;
}
