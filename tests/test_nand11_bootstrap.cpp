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
    
    // Test NAND(1,1)
    GoldenFHE::Cipher one_a = GoldenFHE::encrypt(pk, true, 1000);
    GoldenFHE::Cipher one_b = GoldenFHE::encrypt(pk, true, 2000);
    
    // Direct NAND na walang bootstrapping
    GoldenFHE::Cipher nand_direct = GoldenFHE::nand_gate(one_a, one_b);
    NTL::ZZ_pX noise_direct = nand_direct.c0 + nand_direct.c1 * sk.sk;
    long v_direct = NTL::conv<long>(NTL::coeff(noise_direct, 0));
    std::cout << "NAND(1,1) direct: " << v_direct << " (should be 0)\n";
    
    // NAND na may bootstrapping
    GoldenFHE::Cipher nand_boot = fhe.nand_with_bootstrap(one_a, one_b);
    NTL::ZZ_pX noise_boot = nand_boot.c0 + nand_boot.c1 * sk.sk;
    long v_boot = NTL::conv<long>(NTL::coeff(noise_boot, 0));
    std::cout << "NAND(1,1) bootstrapped: " << v_boot << " (should be 0)\n";
    
    // I-check kung tama ang decryption ng direct NAND
    bool dec_direct = GoldenFHE::decrypt(nand_direct, sk);
    std::cout << "Decrypt direct: " << dec_direct << " (should be 0)\n";
    
    return 0;
}
