#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    // XOR(0,1) = 1
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 2000);
    
    // Step 1: nand_ab = NAND(0,1) = 1
    GoldenFHE::Cipher nand_ab = GoldenFHE::nand_gate(a, b);
    // I-bootstrap para malinis
    nand_ab = boot.bootstrap(nand_ab);
    
    NTL::ZZ_pX noise1 = nand_ab.c0 + nand_ab.c1 * sk.sk;
    long v1 = NTL::conv<long>(NTL::coeff(noise1, 0));
    std::cout << "Step 1 - NAND(0,1) bootstrapped: " << v1 << " (expect " << golden_plain << ")\n";
    
    // Step 2: nand_a_ab = NAND(0, 1) = 1
    GoldenFHE::Cipher nand_a_ab = GoldenFHE::nand_gate(a, nand_ab);
    nand_a_ab = boot.bootstrap(nand_a_ab);
    
    NTL::ZZ_pX noise2 = nand_a_ab.c0 + nand_a_ab.c1 * sk.sk;
    long v2 = NTL::conv<long>(NTL::coeff(noise2, 0));
    std::cout << "Step 2 - NAND(0,1) bootstrapped: " << v2 << " (expect " << golden_plain << ")\n";
    
    // Step 3: nand_b_ab = NAND(1, 1) = 0
    GoldenFHE::Cipher nand_b_ab = GoldenFHE::nand_gate(b, nand_ab);
    nand_b_ab = boot.bootstrap(nand_b_ab);
    
    NTL::ZZ_pX noise3 = nand_b_ab.c0 + nand_b_ab.c1 * sk.sk;
    long v3 = NTL::conv<long>(NTL::coeff(noise3, 0));
    std::cout << "Step 3 - NAND(1,1) bootstrapped: " << v3 << " (expect 0)\n";
    
    // Step 4: xor = NAND(1, 0) = 1
    GoldenFHE::Cipher xor_result = GoldenFHE::nand_gate(nand_a_ab, nand_b_ab);
    xor_result = boot.bootstrap(xor_result);
    
    NTL::ZZ_pX noise4 = xor_result.c0 + xor_result.c1 * sk.sk;
    long v4 = NTL::conv<long>(NTL::coeff(noise4, 0));
    std::cout << "Step 4 - XOR(0,1) bootstrapped: " << v4 << " (expect " << golden_plain << ")\n";
    
    bool final_dec = GoldenFHE::decrypt(xor_result, sk);
    std::cout << "\nFinal XOR(0,1): " << final_dec << " (expected 1)\n";
    
    return 0;
}
