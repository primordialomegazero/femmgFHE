#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto nand00 = fhe.nand_gate(ct0, ct0);
    
    NTL::ZZ_pX noise = nand00.first + nand00.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
    
    std::cout << "v = " << v << "\n";
    std::cout << "golden_plain = " << fhe.golden_plain << "\n";
    std::cout << "Q/2 = " << Q/2 << "\n";
    std::cout << "dist_0 = " << ((v < Q/2) ? v : Q - v) << "\n";
    NTL::ZZ diff = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
    std::cout << "dist_golden = " << ((diff < Q/2) ? diff : Q - diff) << "\n";
    std::cout << "fhe.decrypt = " << fhe.decrypt(nand00) << "\n";
    
    return 0;
}
