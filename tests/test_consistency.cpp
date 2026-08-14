#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    // Test 1: Direct NAND(1,1)
    GoldenFHE::Cipher one = GoldenFHE::encrypt(pk, true, 1000);
    GoldenFHE::Cipher nand11 = GoldenFHE::nand_gate(one, one);
    
    NTL::ZZ_pX noise = nand11.c0 + nand11.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    
    bool dec = GoldenFHE::decrypt(nand11, sk);
    
    std::cout << "Test 1 - NAND(1,1):\n";
    std::cout << "  Value: " << v << "\n";
    std::cout << "  Threshold: " << threshold << "\n";
    std::cout << "  Decrypt result: " << dec << " (expected 0)\n\n";
    
    // Test 2: NAND(0,1)
    GoldenFHE::Cipher zero = GoldenFHE::encrypt(pk, false, 2000);
    GoldenFHE::Cipher one2 = GoldenFHE::encrypt(pk, true, 3000);
    GoldenFHE::Cipher nand01 = GoldenFHE::nand_gate(zero, one2);
    
    NTL::ZZ_pX noise2 = nand01.c0 + nand01.c1 * sk.sk;
    long v2 = NTL::conv<long>(NTL::coeff(noise2, 0));
    
    bool dec2 = GoldenFHE::decrypt(nand01, sk);
    
    std::cout << "Test 2 - NAND(0,1):\n";
    std::cout << "  Value: " << v2 << "\n";
    std::cout << "  Threshold: " << threshold << "\n";
    std::cout << "  Decrypt result: " << dec2 << " (expected 1)\n";
    
    return 0;
}
