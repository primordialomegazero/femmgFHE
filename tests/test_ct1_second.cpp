#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    
    // I-check ang ct1.second coefficients
    std::cout << "ct1.second degree: " << NTL::deg(ct1.second) << "\n";
    std::cout << "ct1.second coeff 0: " << NTL::coeff(ct1.second, 0) << "\n";
    std::cout << "ct1.second coeff 1: " << NTL::coeff(ct1.second, 1) << "\n\n";
    
    // I-check ang ct1.first
    std::cout << "ct1.first degree: " << NTL::deg(ct1.first) << "\n";
    std::cout << "ct1.first coeff 0: " << NTL::coeff(ct1.first, 0) << "\n\n";
    
    // I-check ang pk1
    std::cout << "pk1 degree: " << NTL::deg(fhe.pk1) << "\n";
    std::cout << "pk1 coeff 0: " << NTL::coeff(fhe.pk1, 0) << "\n";
    std::cout << "pk1 coeff 1: " << NTL::coeff(fhe.pk1, 1) << "\n\n";
    
    // I-check ang u na ginamit sa encryption
    // (Hindi natin ma-access directly, pero pwede nating i-check ang encryption params)
    std::cout << "s_val = " << fhe.s_val << "\n";
    
    return 0;
}
