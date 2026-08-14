#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Test nonce 1000000 (problematic)
    GoldenFHE::Cipher one1 = GoldenFHE::encrypt(pk, true, 1000000);
    GoldenFHE::Cipher one2 = GoldenFHE::encrypt(pk, true, 1000001);
    
    // I-compute ang s² term
    NTL::ZZ_pX s = sk.sk;
    NTL::ZZ_pX s_squared = s * s;
    
    // I-check ang coefficient 0 ng s²
    NTL::ZZ_pX noise_s2 = s_squared;
    long v_s2 = NTL::conv<long>(NTL::coeff(noise_s2, 0));
    
    std::cout << "s^2 coefficient 0: " << v_s2 << "\n";
    std::cout << "s^2 mod Q: " << (v_s2 % GoldenFHE::Q) << "\n";
    
    // Ang tamang t2 term
    NTL::ZZ_pX t2 = one1.c1 * one2.c1;
    NTL::ZZ_pX noise_t2 = t2 * s_squared;
    long v_t2 = NTL::conv<long>(NTL::coeff(noise_t2, 0));
    
    std::cout << "t2 * s^2: " << v_t2 << "\n";
    
    // Ang current approximation: t2 * (-1)
    NTL::ZZ_pX neg_one;
    NTL::SetCoeff(neg_one, 0, -1);
    NTL::ZZ_pX t2_approx = t2 * neg_one;
    NTL::ZZ_pX noise_approx = t2_approx;
    long v_approx = NTL::conv<long>(NTL::coeff(noise_approx, 0));
    
    std::cout << "t2 * (-1): " << v_approx << "\n";
    std::cout << "Difference: " << (v_t2 - v_approx) << "\n";
    
    return 0;
}
