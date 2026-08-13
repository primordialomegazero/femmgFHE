#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Encrypt 1 and 1
    GoldenFHE::Cipher ct1 = GoldenFHE::encrypt(pk, true, 200);
    GoldenFHE::Cipher ct1b = GoldenFHE::encrypt(pk, true, 300);
    
    // Check individual decryptions
    NTL::ZZ_pX noise1 = ct1.c0 + ct1.c1 * sk.sk;
    long v1 = NTL::conv<long>(NTL::coeff(noise1, 0));
    std::cout << "ct1 value: " << v1 << " (expected ~" << golden_plain << ")\n";
    
    // Multiplication
    NTL::ZZ_pX t0 = ct1.c0 * ct1b.c0;
    NTL::ZZ_pX t1 = ct1.c0 * ct1b.c1 + ct1.c1 * ct1b.c0;
    NTL::ZZ_pX t2 = ct1.c1 * ct1b.c1;
    
    NTL::ZZ_pX mult_c0 = t0 - t2;
    NTL::ZZ_pX mult_c1 = t1;
    
    NTL::ZZ_pX noise_mult = mult_c0 + mult_c1 * sk.sk;
    long v_mult = NTL::conv<long>(NTL::coeff(noise_mult, 0));
    
    std::cout << "\nMultiplication (1*1): " << v_mult << "\n";
    std::cout << "Expected: ~" << (golden_plain * golden_plain) % GoldenFHE::Q << " or scaled value\n";
    std::cout << "golden_plain^2 mod Q: " << (golden_plain * golden_plain) % GoldenFHE::Q << "\n";
    std::cout << "golden_plain^2: " << (golden_plain * golden_plain) << "\n";
    
    // NAND = golden_plain - mult
    NTL::ZZ_pX golden_poly;
    NTL::SetCoeff(golden_poly, 0, golden_plain);
    
    NTL::ZZ_pX nand_c0 = golden_poly - mult_c0;
    NTL::ZZ_pX nand_c1 = -mult_c1;
    
    NTL::ZZ_pX noise_nand = nand_c0 + nand_c1 * sk.sk;
    long v_nand = NTL::conv<long>(NTL::coeff(noise_nand, 0));
    
    std::cout << "\nNAND(1,1) value: " << v_nand << "\n";
    std::cout << "Threshold: " << static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI)) << "\n";
    std::cout << "Should be < threshold for 0\n";
    
    return 0;
}
