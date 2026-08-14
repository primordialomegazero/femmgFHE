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
    
    // Test XOR(0,1)
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 2000);
    
    // Step by step
    GoldenFHE::Cipher nand_ab = fhe.nand_with_bootstrap(a, b);
    NTL::ZZ_pX noise_nand_ab = nand_ab.c0 + nand_ab.c1 * sk.sk;
    long v_nand_ab = NTL::conv<long>(NTL::coeff(noise_nand_ab, 0));
    std::cout << "NAND(0,1): " << v_nand_ab << " (should be " << golden_plain << ")\n";
    
    GoldenFHE::Cipher nand_a_ab = fhe.nand_with_bootstrap(a, nand_ab);
    NTL::ZZ_pX noise_nand_a_ab = nand_a_ab.c0 + nand_a_ab.c1 * sk.sk;
    long v_nand_a_ab = NTL::conv<long>(NTL::coeff(noise_nand_a_ab, 0));
    std::cout << "NAND(0, NAND(0,1)): " << v_nand_a_ab << " (should be " << golden_plain << ")\n";
    
    GoldenFHE::Cipher nand_b_ab = fhe.nand_with_bootstrap(b, nand_ab);
    NTL::ZZ_pX noise_nand_b_ab = nand_b_ab.c0 + nand_b_ab.c1 * sk.sk;
    long v_nand_b_ab = NTL::conv<long>(NTL::coeff(noise_nand_b_ab, 0));
    std::cout << "NAND(1, NAND(0,1)): " << v_nand_b_ab << " (should be " << golden_plain << ")\n";
    
    GoldenFHE::Cipher xor_result = fhe.nand_with_bootstrap(nand_a_ab, nand_b_ab);
    NTL::ZZ_pX noise_xor = xor_result.c0 + xor_result.c1 * sk.sk;
    long v_xor = NTL::conv<long>(NTL::coeff(noise_xor, 0));
    std::cout << "XOR(0,1): " << v_xor << " (should be " << golden_plain << ")\n";
    
    return 0;
}
