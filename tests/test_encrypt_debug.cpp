#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    // Check golden_plain_p
    std::cout << "golden_plain_p = " << fhe.golden_plain_p << "\n";
    std::cout << "golden_plain = " << fhe.golden_plain << "\n\n";
    
    // Manually create ciphertext for bit=1
    NTL::ZZ_pX m;
    NTL::SetCoeff(m, 0, fhe.golden_plain_p);
    std::cout << "m (manual) = " << NTL::coeff(m, 0) << "\n";
    
    // Test encrypt_internal directly
    auto ct1 = fhe.encrypt_internal(true, NTL::to_ZZ(12345));
    
    // Decrypt manually
    NTL::ZZ_pX noise = ct1.first + ct1.second * fhe.s;
    fhe.reduce_mod(noise);
    NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
    std::cout << "Noise value: " << v << "\n";
    std::cout << "Should be ≈ golden_plain: " << fhe.golden_plain << "\n";
    
    return 0;
}
