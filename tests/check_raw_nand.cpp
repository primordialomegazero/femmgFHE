#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto result = fhe.raw_nand(ct0, ct0);
    
    NTL::ZZ_pX noise = result.first + result.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
    
    std::cout << "raw_nand(0,0) v = " << v << "\n";
    std::cout << "dist_0 = " << ((v < Q/2) ? v : Q - v) << "\n";
    NTL::ZZ diff = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
    std::cout << "dist_golden = " << ((diff < Q/2) ? diff : Q - diff) << "\n";
    std::cout << "decrypt = " << fhe.decrypt(result) << "\n";
    
    // Nand_gate naman
    auto result2 = fhe.nand_gate(ct0, ct0);
    NTL::ZZ_pX noise2 = result2.first + result2.second * fhe.s;
    fhe.reduce_mod(noise2);
    NTL::ZZ v2 = NTL::rep(NTL::coeff(noise2, 0));
    
    std::cout << "\nnand_gate(0,0) v = " << v2 << "\n";
    std::cout << "dist_0 = " << ((v2 < Q/2) ? v2 : Q - v2) << "\n";
    NTL::ZZ diff2 = (v2 > fhe.golden_plain) ? v2 - fhe.golden_plain : fhe.golden_plain - v2;
    std::cout << "dist_golden = " << ((diff2 < Q/2) ? diff2 : Q - diff2) << "\n";
    std::cout << "decrypt = " << fhe.decrypt(result2) << "\n";
    
    return 0;
}
