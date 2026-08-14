#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk, 1);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    // Test: NAND(1, 1) dapat = 0
    GoldenFHE::Cipher one_a = GoldenFHE::encrypt(pk, true, 1000);
    GoldenFHE::Cipher one_b = GoldenFHE::encrypt(pk, true, 2000);
    
    // Direct NAND walang bootstrapping
    GoldenFHE::Cipher nand_direct = GoldenFHE::nand_gate(one_a, one_b);
    NTL::ZZ_pX noise_direct = nand_direct.c0 + nand_direct.c1 * sk.sk;
    long v_direct = NTL::conv<long>(NTL::coeff(noise_direct, 0));
    std::cout << "NAND(1,1) direct: " << v_direct << "\n";
    std::cout << "Threshold: " << threshold << "\n";
    std::cout << "Decrypt: " << (v_direct > threshold) << "\n\n";
    
    // NAND(1,1) na may bootstrap every 1
    GoldenFHE::Cipher nand_boot = fhe.nand_with_bootstrap(one_a, one_b);
    NTL::ZZ_pX noise_boot = nand_boot.c0 + nand_boot.c1 * sk.sk;
    long v_boot = NTL::conv<long>(NTL::coeff(noise_boot, 0));
    std::cout << "NAND(1,1) bootstrapped: " << v_boot << "\n";
    std::cout << "Decrypt: " << (v_boot > threshold) << "\n\n";
    
    // Ngayon, ano ang nangyayari sa NAND(0,1)?
    GoldenFHE::Cipher zero = GoldenFHE::encrypt(pk, false, 3000);
    GoldenFHE::Cipher one = GoldenFHE::encrypt(pk, true, 4000);
    
    GoldenFHE::Cipher nand01_boot = fhe.nand_with_bootstrap(zero, one);
    NTL::ZZ_pX noise01 = nand01_boot.c0 + nand01_boot.c1 * sk.sk;
    long v01 = NTL::conv<long>(NTL::coeff(noise01, 0));
    std::cout << "NAND(0,1) bootstrapped: " << v01 << "\n";
    std::cout << "Decrypt: " << (v01 > threshold) << "\n";
    
    return 0;
}
