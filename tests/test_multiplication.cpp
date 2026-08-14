#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    // Test 1*1
    GoldenFHE::Cipher one_a = GoldenFHE::encrypt(pk, true, 1000);
    GoldenFHE::Cipher one_b = GoldenFHE::encrypt(pk, true, 2000);
    
    // Multiply
    NTL::ZZ_pX t0 = one_a.c0 * one_b.c0;
    NTL::ZZ_pX t1 = one_a.c0 * one_b.c1 + one_a.c1 * one_b.c0;
    NTL::ZZ_pX t2 = one_a.c1 * one_b.c1;
    
    NTL::ZZ_pX mult_c0 = t0 - t2;
    NTL::ZZ_pX mult_c1 = t1;
    
    NTL::ZZ_pX noise_mult = mult_c0 + mult_c1 * sk.sk;
    long v_mult = NTL::conv<long>(NTL::coeff(noise_mult, 0));
    
    std::cout << "1*1 multiplication: " << v_mult << "\n";
    std::cout << "golden_plain^2 mod Q: " << (golden_plain * golden_plain) % GoldenFHE::Q << "\n";
    
    // Ang 1*1 ay dapat golden_plain^2 mod Q = 137048910
    // Pero ang tamang NAND(1,1) = 0 ay dapat magbigay ng value < threshold
    
    // Subukan nating gamitin ang tamang formula:
    // NAND = golden_plain - (mult / golden_plain)
    NTL::ZZ_p inv_golden;
    inv_golden = golden_plain;
    NTL::ZZ_p inv_val = NTL::inv(inv_golden);
    long inv_long = NTL::conv<long>(inv_val);
    
    NTL::ZZ_pX scaled_c0 = mult_c0 * inv_long;
    NTL::ZZ_pX scaled_c1 = mult_c1 * inv_long;
    
    NTL::ZZ_pX golden_poly;
    NTL::SetCoeff(golden_poly, 0, golden_plain);
    
    NTL::ZZ_pX nand_c0 = golden_poly - scaled_c0;
    NTL::ZZ_pX nand_c1 = -scaled_c1;
    
    NTL::ZZ_pX noise_nand = nand_c0 + nand_c1 * sk.sk;
    long v_nand = NTL::conv<long>(NTL::coeff(noise_nand, 0));
    
    std::cout << "NAND(1,1): " << v_nand << " (should be < " << threshold << ")\n";
    std::cout << "Difference from 0: " << v_nand << "\n";
    
    return 0;
}
