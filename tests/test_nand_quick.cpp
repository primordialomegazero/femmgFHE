#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    auto nand11 = fhe.nand_gate(ct1, ct1);
    
    std::cout << "NAND(1,1) = " << fhe.decrypt(nand11) << " (exp 0)\n";
    
    // Manual check
    NTL::ZZ_pX noise = nand11.first + nand11.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
    std::cout << "v = " << v << "\n";
    std::cout << "golden_plain = " << fhe.golden_plain << "\n";
    std::cout << "dist_0 = " << ((v < Q/2) ? v : Q - v) << "\n";
    NTL::ZZ diff = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
    std::cout << "dist_golden = " << ((diff < Q/2) ? diff : Q - diff) << "\n";
    
    return 0;
}
