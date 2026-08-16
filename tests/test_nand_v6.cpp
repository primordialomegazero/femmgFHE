#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "NAND V6 DEEP DEBUG\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    auto nand11 = fhe.nand_gate(ct1, ct1);
    
    // Manual decrypt
    NTL::ZZ_pX noise = nand11.first + nand11.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
    
    NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
    NTL::ZZ diff = (v > fhe.golden_plain) ? v - fhe.golden_plain : fhe.golden_plain - v;
    NTL::ZZ dist_golden = (diff < Q/2) ? diff : Q - diff;
    
    std::cout << "v = " << v << "\n";
    std::cout << "Q = " << Q << "\n";
    std::cout << "golden_plain = " << fhe.golden_plain << "\n";
    std::cout << "dist_0 = " << dist_0 << "\n";
    std::cout << "dist_golden = " << dist_golden << "\n";
    std::cout << "Decision: " << (dist_golden < dist_0 ? "1" : "0") << "\n";
    std::cout << "fhe.decrypt: " << fhe.decrypt(nand11) << "\n";
    
    return 0;
}
