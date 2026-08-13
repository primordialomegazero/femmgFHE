#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>
#include <cassert>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    // Test basic NAND
    GoldenFHE::Cipher ct0 = GoldenFHE::encrypt(pk, false, 100);
    GoldenFHE::Cipher ct1 = GoldenFHE::encrypt(pk, true, 200);
    
    // NAND(0,1) = 1
    GoldenFHE::Cipher nand01 = GoldenFHE::nand_gate(ct0, ct1);
    bool dec_nand01 = GoldenFHE::decrypt(nand01, sk);
    std::cout << "NAND(0,1) = " << dec_nand01 << " (expected 1)\n";
    assert(dec_nand01 == true);
    
    // NAND(1,1) = 0
    GoldenFHE::Cipher ct1b = GoldenFHE::encrypt(pk, true, 300);
    GoldenFHE::Cipher nand11 = GoldenFHE::nand_gate(ct1, ct1b);
    bool dec_nand11 = GoldenFHE::decrypt(nand11, sk);
    std::cout << "NAND(1,1) = " << dec_nand11 << " (expected 0)\n";
    assert(dec_nand11 == false);
    
    // Test XOR
    GoldenFHE::Cipher xor01 = GoldenFHE::XOR(ct0, ct1);
    bool dec_xor01 = GoldenFHE::decrypt(xor01, sk);
    std::cout << "XOR(0,1) = " << dec_xor01 << " (expected 1)\n";
    assert(dec_xor01 == true);
    
    // Test AND
    GoldenFHE::Cipher and11 = GoldenFHE::AND(ct1, ct1b);
    bool dec_and11 = GoldenFHE::decrypt(and11, sk);
    std::cout << "AND(1,1) = " << dec_and11 << " (expected 1)\n";
    assert(dec_and11 == true);
    
    std::cout << "\n✅ ALL TESTS PASSED!\n";
    return 0;
}
