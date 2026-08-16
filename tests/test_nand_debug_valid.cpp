#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "NAND DEBUG - V6 WITH VALID Q\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    auto nand11 = fhe.nand_gate(ct1, ct1);
    
    // Manual decryption
    NTL::ZZ_pX noise = nand11.first + nand11.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
    
    NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
    NTL::ZZ diff = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
    NTL::ZZ dist_golden = (diff < Q/2) ? diff : Q - diff;
    
    std::cout << "v = " << v << "\n";
    std::cout << "Q/2 = " << Q/2 << "\n";
    std::cout << "golden_plain = " << fhe.golden_plain << "\n";
    std::cout << "dist_0 = " << dist_0 << "\n";
    std::cout << "dist_golden = " << dist_golden << "\n";
    std::cout << "Decision: " << (dist_golden < dist_0 ? "1" : "0") << "\n";
    std::cout << "fhe.decrypt: " << fhe.decrypt(nand11) << "\n";
    
    // I-check ang ct1 mismo
    NTL::ZZ_pX noise1 = ct1.first + ct1.second * fhe.s;
    fhe.reduce_mod(noise1);
    NTL::ZZ v1 = NTL::rep(NTL::coeff(noise1, 0));
    std::cout << "\nct1 v = " << v1 << "\n";
    std::cout << "ct1 dist_0 = " << ((v1 < Q/2) ? v1 : Q - v1) << "\n";
    NTL::ZZ diff1 = (v1 > fhe.golden_plain) ? v1 - fhe.golden_plain : fhe.golden_plain - v1;
    std::cout << "ct1 dist_golden = " << ((diff1 < Q/2) ? diff1 : Q - diff1) << "\n";
    
    return 0;
}
