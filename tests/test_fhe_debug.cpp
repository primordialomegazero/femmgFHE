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
    
    // I-print ang decrypted values
    NTL::ZZ_pX noise0 = ct0.c0 + ct0.c1 * sk.sk;
    long v0 = NTL::conv<long>(NTL::coeff(noise0, 0));
    
    NTL::ZZ_pX noise1 = ct1.c0 + ct1.c1 * sk.sk;
    long v1 = NTL::conv<long>(NTL::coeff(noise1, 0));
    
    std::cout << "Plaintext 0 value: " << v0 << "\n";
    std::cout << "Plaintext 1 value: " << v1 << "\n";
    std::cout << "Threshold: " << static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI)) << "\n";
    std::cout << "Q/PHI: " << static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI) << "\n";
    std::cout << "Q/2: " << GoldenFHE::Q / 2 << "\n";
    
    // Test NAND directly
    GoldenFHE::Cipher nand01 = GoldenFHE::nand_gate(ct0, ct1);
    NTL::ZZ_pX noise_nand = nand01.c0 + nand01.c1 * sk.sk;
    long v_nand = NTL::conv<long>(NTL::coeff(noise_nand, 0));
    std::cout << "NAND value: " << v_nand << "\n";
    
    // Subukan iba't ibang thresholds
    for (long t = 0; t < GoldenFHE::Q; t += GoldenFHE::Q/10) {
        std::cout << "Threshold " << t << ": " << (v_nand > t) << "\n";
    }
    
    return 0;
}
