#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    // Test ang direct formula sa pamamagitan ng φ-normalization
    // NAND(0,0) = φ - 0·0·φ⁻¹ = φ
    // Kaya dapat ang result ay may v = φ
    
    auto nand00 = fhe.nand_gate(ct0, ct0);
    
    NTL::ZZ_pX noise = nand00.first + nand00.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
    
    std::cout << "NAND(0,0) v = " << v << "\n";
    std::cout << "φ = " << fhe.golden_plain << "\n";
    
    // Kung v malapit sa 0, kailangan nating i-normalize
    if (v < Q/2) {
        // I-normalize: v = φ - v (para mapunta sa golden_plain orbit)
        NTL::ZZ corrected = (fhe.golden_plain - v) % Q;
        if (corrected < 0) corrected += Q;
        std::cout << "Corrected v = " << corrected << "\n";
        std::cout << "Corrected decrypt: " << (corrected > Q/2 ? 0 : 1) << "\n";
    }
    
    return 0;
}
