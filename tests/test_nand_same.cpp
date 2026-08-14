#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Test NAND na parehong inputs
    GoldenFHE::Cipher one = GoldenFHE::encrypt(pk, true, 1000000);
    
    GoldenFHE::Cipher nand_same = GoldenFHE::nand_gate(one, one);
    
    NTL::ZZ_pX noise = nand_same.c0 + nand_same.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    bool dec = GoldenFHE::decrypt(nand_same, sk);
    
    std::cout << "NAND(1,1) same ciphertext: " << v << " dec=" << dec << " (expect 0)\n";
    
    // Ngayon test na magkaibang ciphertext pero parehong value 1
    GoldenFHE::Cipher one2 = GoldenFHE::encrypt(pk, true, 1100000);
    
    GoldenFHE::Cipher nand_diff = GoldenFHE::nand_gate(one, one2);
    
    NTL::ZZ_pX noise2 = nand_diff.c0 + nand_diff.c1 * sk.sk;
    long v2 = NTL::conv<long>(NTL::coeff(noise2, 0));
    bool dec2 = GoldenFHE::decrypt(nand_diff, sk);
    
    std::cout << "NAND(1,1) diff ciphertext: " << v2 << " dec=" << dec2 << " (expect 0)\n";
    
    return 0;
}
