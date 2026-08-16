#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "φ-NORMALIZATION TEST\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    
    // I-convert ang phi_zz sa ZZ_p
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(fhe.phi_zz);
    
    // Manual φ-normalization
    auto normalize = [&](const golden_fhe_v6::FibonacciFHEV6::Cipher& ct) {
        auto result = ct;
        result.first = result.first * phi_p;
        result.second = result.second * phi_p;
        fhe.reduce_mod(result.first);
        fhe.reduce_mod(result.second);
        return result;
    };
    
    auto current = normalize(ct1);
    
    for (int i = 1; i <= 10; i++) {
        current = fhe.nand_gate(current, current);
        bool result = fhe.decrypt(current);
        bool expected = (i % 2 == 0);
        std::cout << "NAND[" << i << "]: " << result 
                  << " (exp " << expected << ") " 
                  << (result == expected ? "✓" : "✗") << "\n";
    }
    
    return 0;
}
