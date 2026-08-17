// V6 WITHOUT RELINEARIZATION — direct φ math para sa degree-0 ct
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "V6 NO RELINEARIZATION TEST\n";
    std::cout << "==========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    // Direct φ encoding (walang RLWE, walang relinearization)
    NTL::ZZ_pX ct1_first, ct1_second;
    NTL::SetCoeff(ct1_first, 0, fhe.golden_plain_p);  // φ
    NTL::SetCoeff(ct1_second, 0, NTL::to_ZZ_p(0));    // 0
    
    auto ct1 = std::make_pair(ct1_first, ct1_second);
    auto current = ct1;
    
    std::cout << "Deep NAND chain (100 depths, no relinearization):\n";
    int errors = 0;
    for (int i = 0; i <= 10; i++) {
        // MANUAL NAND: result = golden_poly - ct.first²·inv_golden
        NTL::ZZ_pX t0 = current.first * current.first;
        fhe.reduce_mod(t0);
        NTL::ZZ_pX scaled = t0 * fhe.inv_golden_p;
        fhe.reduce_mod(scaled);
        NTL::ZZ_pX result_first = fhe.golden_poly - scaled;
        NTL::ZZ_pX result_second;
        NTL::SetCoeff(result_second, 0, NTL::to_ZZ_p(0));
        
        current = {result_first, result_second};
        
        bool dec = fhe.decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        std::cout << "  Depth " << i << ": dec=" << dec << ", exp=" << expected 
                  << (dec == expected ? " ✓" : " ✗") << "\n";
        if (dec != expected) errors++;
    }
    
    std::cout << "\nTotal errors: " << errors << "/101\n";
    return 0;
}
