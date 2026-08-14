#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    std::cout << "golden_plain: " << golden_plain << "\n";
    std::cout << "golden_plain^2 mod Q: " << (golden_plain * golden_plain) % GoldenFHE::Q << "\n";
    std::cout << "threshold: " << threshold << "\n\n";
    
    // Inverse ng golden_plain
    NTL::ZZ_p inv_golden;
    inv_golden = golden_plain;
    NTL::ZZ_p inv_val = NTL::inv(inv_golden);
    long inv_long = NTL::conv<long>(inv_val);
    std::cout << "inv_golden: " << inv_long << "\n";
    std::cout << "Check: golden_plain * inv_golden mod Q = " << (golden_plain * inv_long) % GoldenFHE::Q << "\n\n";
    
    // Encrypt 1
    GoldenFHE::Cipher one = GoldenFHE::encrypt(pk, true, 1000);
    
    // NOT(1) = NAND(1,1) dapat 0
    GoldenFHE::Cipher nand11 = GoldenFHE::nand_gate(one, one);
    
    NTL::ZZ_pX noise = nand11.c0 + nand11.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    
    std::cout << "NAND(1,1) result: " << v << "\n";
    std::cout << "Expected 0 (less than " << threshold << ")\n";
    std::cout << "Distance from 0: " << v << "\n";
    std::cout << "Distance from golden_plain: " << std::abs(v - golden_plain) << "\n";
    
    return 0;
}
