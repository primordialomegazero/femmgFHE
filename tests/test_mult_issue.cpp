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
    
    // I-check ang individual values
    NTL::ZZ_pX noise1 = one1.c0 + one1.c1 * sk.sk;
    long v1 = NTL::conv<long>(NTL::coeff(noise1, 0));
    
    NTL::ZZ_pX noise2 = one2.c0 + one2.c1 * sk.sk;
    long v2 = NTL::conv<long>(NTL::coeff(noise2, 0));
    
    std::cout << "one1: " << v1 << " (expect " << golden_plain << ")\n";
    std::cout << "one2: " << v2 << " (expect " << golden_plain << ")\n";
    
    // Multiplication
    NTL::ZZ_pX t0 = one1.c0 * one2.c0;
    NTL::ZZ_pX t1 = one1.c0 * one2.c1 + one1.c1 * one2.c0;
    NTL::ZZ_pX t2 = one1.c1 * one2.c1;
    
    NTL::ZZ_pX mult_c0 = t0 - t2;
    NTL::ZZ_pX mult_c1 = t1;
    
    NTL::ZZ_pX noise_mult = mult_c0 + mult_c1 * sk.sk;
    long v_mult = NTL::conv<long>(NTL::coeff(noise_mult, 0));
    
    std::cout << "mult: " << v_mult << " (expect " << (golden_plain * golden_plain) % GoldenFHE::Q << ")\n";
    
    // Test nonce 1000010 (working)
    GoldenFHE::Cipher one3 = GoldenFHE::encrypt(pk, true, 1000010);
    GoldenFHE::Cipher one4 = GoldenFHE::encrypt(pk, true, 1000011);
    
    NTL::ZZ_pX noise3 = one3.c0 + one3.c1 * sk.sk;
    long v3 = NTL::conv<long>(NTL::coeff(noise3, 0));
    
    NTL::ZZ_pX noise4 = one4.c0 + one4.c1 * sk.sk;
    long v4 = NTL::conv<long>(NTL::coeff(noise4, 0));
    
    std::cout << "\none3: " << v3 << " (expect " << golden_plain << ")\n";
    std::cout << "one4: " << v4 << " (expect " << golden_plain << ")\n";
    
    NTL::ZZ_pX t0b = one3.c0 * one4.c0;
    NTL::ZZ_pX t1b = one3.c0 * one4.c1 + one3.c1 * one4.c0;
    NTL::ZZ_pX t2b = one3.c1 * one4.c1;
    
    NTL::ZZ_pX mult_c0b = t0b - t2b;
    NTL::ZZ_pX mult_c1b = t1b;
    
    NTL::ZZ_pX noise_multb = mult_c0b + mult_c1b * sk.sk;
    long v_multb = NTL::conv<long>(NTL::coeff(noise_multb, 0));
    
    std::cout << "mult: " << v_multb << " (expect " << (golden_plain * golden_plain) % GoldenFHE::Q << ")\n";
    
    return 0;
}
