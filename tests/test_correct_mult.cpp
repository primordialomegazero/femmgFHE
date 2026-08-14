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
    
    // Tamang multiplication:
    // (c0 + c1*s) * (d0 + d1*s) = c0*d0 + (c0*d1 + c1*d0)*s + c1*d1*s²
    // Kaya: new_c0 = c0*d0, new_c1 = c0*d1 + c1*d0, new_c2 = c1*d1
    
    NTL::ZZ_pX new_c0 = one1.c0 * one2.c0;
    NTL::ZZ_pX new_c1 = one1.c0 * one2.c1 + one1.c1 * one2.c0;
    NTL::ZZ_pX new_c2 = one1.c1 * one2.c1;
    
    // Ang decryption: (new_c0 + new_c1*s + new_c2*s²)
    NTL::ZZ_pX s = sk.sk;
    NTL::ZZ_pX s2 = s * s;
    
    NTL::ZZ_pX decryption = new_c0 + new_c1 * s + new_c2 * s2;
    long v = NTL::conv<long>(NTL::coeff(decryption, 0));
    
    std::cout << "Correct mult result: " << v << "\n";
    std::cout << "Expected: " << (golden_plain * golden_plain) % GoldenFHE::Q << "\n";
    
    // Ang current approximation: s² = -1
    NTL::ZZ_pX decryption_approx = new_c0 + new_c1 * s - new_c2;
    long v_approx = NTL::conv<long>(NTL::coeff(decryption_approx, 0));
    
    std::cout << "Approx mult result: " << v_approx << "\n";
    
    return 0;
}
