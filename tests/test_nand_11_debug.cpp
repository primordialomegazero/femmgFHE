// Debug NAND(1,1)
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    
    // Step-by-step NAND(1,1)
    NTL::ZZ_pX t0 = ct1.first * ct1.first;
    NTL::ZZ_pX t1 = ct1.first * ct1.second + ct1.second * ct1.first;
    NTL::ZZ_pX t2 = ct1.second * ct1.second;
    fhe.reduce_mod(t0); fhe.reduce_mod(t1); fhe.reduce_mod(t2);
    
    std::cout << "t0 coeff 0 = " << NTL::coeff(t0, 0) << "\n";
    std::cout << "t1 coeff 0 = " << NTL::coeff(t1, 0) << "\n";
    std::cout << "t2 coeff 0 = " << NTL::coeff(t2, 0) << "\n\n";
    
    NTL::ZZ_pX mult_c0 = t0 + t2 * fhe.beta_p;
    NTL::ZZ_pX mult_c1 = t1 + t2 * fhe.alpha_p;
    fhe.reduce_mod(mult_c0); fhe.reduce_mod(mult_c1);
    
    std::cout << "mult_c0 coeff 0 = " << NTL::coeff(mult_c0, 0) << "\n";
    std::cout << "mult_c1 coeff 0 = " << NTL::coeff(mult_c1, 0) << "\n\n";
    
    // Result = golden_poly - mult_c0
    NTL::ZZ_pX result_c0 = fhe.golden_poly - mult_c0;
    NTL::ZZ_pX result_c1 = -mult_c1;
    
    std::cout << "result_c0 coeff 0 = " << NTL::coeff(result_c0, 0) << "\n";
    std::cout << "result_c1 coeff 0 = " << NTL::coeff(result_c1, 0) << "\n\n";
    
    // Decrypt
    NTL::ZZ_pX noise = result_c0 + result_c1 * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
    std::cout << "Noise v = " << v << "\n";
    std::cout << "Expected: 0 (NAND(1,1) = 0)\n";
    
    return 0;
}
