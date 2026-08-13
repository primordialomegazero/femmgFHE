#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    // Encrypt 1
    GoldenFHE::Cipher one = GoldenFHE::encrypt(pk, true, 1000);
    
    // NOT(1) = 0
    GoldenFHE::Cipher zero = GoldenFHE::NOT(one);
    
    // I-check ang decryption ng zero
    NTL::ZZ_pX noise_zero = zero.c0 + zero.c1 * sk.sk;
    long v_zero = NTL::conv<long>(NTL::coeff(noise_zero, 0));
    
    // I-check ang decryption ng one
    NTL::ZZ_pX noise_one = one.c0 + one.c1 * sk.sk;
    long v_one = NTL::conv<long>(NTL::coeff(noise_one, 0));
    
    std::cout << "Golden plain (encrypted 1): " << golden_plain << "\n";
    std::cout << "Actual encrypted 1: " << v_one << "\n";
    std::cout << "NOT(1) should be 0: " << v_zero << "\n";
    std::cout << "Threshold: " << threshold << "\n\n";
    
    // Ang problema: ang NAND formula ay gumagamit ng literal 1, hindi golden_plain
    // Kaya ang result ay maliit na value, hindi malapit sa 0 o golden_plain
    
    // Subukan natin ang tamang formula: NAND(a,b) = golden_plain - a*b/golden_plain
    NTL::ZZ_pX t0 = one.c0 * one.c0;
    NTL::ZZ_pX t1 = one.c0 * one.c1 + one.c1 * one.c0;
    NTL::ZZ_pX t2 = one.c1 * one.c1;
    
    NTL::ZZ_pX mult_c0 = t0 - t2;
    NTL::ZZ_pX mult_c1 = t1;
    
    // I-rescale
    NTL::ZZ_p inv_golden;
    inv_golden = golden_plain;
    NTL::ZZ_p inv_val = NTL::inv(inv_golden);
    long inv_long = NTL::conv<long>(inv_val);
    
    NTL::ZZ_pX scaled_c0 = mult_c0 * inv_long;
    NTL::ZZ_pX scaled_c1 = mult_c1 * inv_long;
    
    // NAND = golden_plain - scaled
    NTL::ZZ_pX gp;
    NTL::SetCoeff(gp, 0, golden_plain);
    
    NTL::ZZ_pX nand_c0 = gp - scaled_c0;
    NTL::ZZ_pX nand_c1 = -scaled_c1;
    
    NTL::ZZ_pX noise_nand = nand_c0 + nand_c1 * sk.sk;
    long v_nand = NTL::conv<long>(NTL::coeff(noise_nand, 0));
    
    std::cout << "Correct NAND formula: " << v_nand << "\n";
    std::cout << "Should be 0 (less than threshold)\n";
    
    return 0;
}
