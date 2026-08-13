#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    // Test basic encryption
    GoldenFHE::Cipher ct0 = GoldenFHE::encrypt(pk, false, 100);
    GoldenFHE::Cipher ct1 = GoldenFHE::encrypt(pk, true, 200);
    
    // I-check ang multiplication
    NTL::ZZ_pX t0 = ct0.c0 * ct1.c0;
    NTL::ZZ_pX t1 = ct0.c0 * ct1.c1 + ct0.c1 * ct1.c0;
    NTL::ZZ_pX t2 = ct0.c1 * ct1.c1;
    
    // I-check ang decryption ng multiplication
    NTL::ZZ_pX mult_c0 = t0 - t2;
    NTL::ZZ_pX mult_c1 = t1;
    
    NTL::ZZ_pX noise_mult = mult_c0 + mult_c1 * sk.sk;
    long v_mult = NTL::conv<long>(NTL::coeff(noise_mult, 0));
    
    std::cout << "Multiplication result: " << v_mult << "\n";
    std::cout << "Expected (0*1=0): 0\n\n";
    
    // Ang NAND ay 1 - mult
    NTL::ZZ_pX one;
    NTL::SetCoeff(one, 0, 1);
    
    NTL::ZZ_pX nand_c0 = one - (t0 - t2);
    NTL::ZZ_pX nand_c1 = -(t1);
    
    NTL::ZZ_pX noise_nand = nand_c0 + nand_c1 * sk.sk;
    long v_nand = NTL::conv<long>(NTL::coeff(noise_nand, 0));
    
    std::cout << "NAND result: " << v_nand << "\n";
    std::cout << "Expected (1-0=1): malapit sa Q/PHI\n";
    
    return 0;
}
