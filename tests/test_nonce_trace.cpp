#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Test (1,1,1)
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, true, 1000000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 1100000);
    GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, true, 1200000);
    
    // XOR(1,1)
    GoldenFHE::Cipher nand_ab = boot.bootstrap(GoldenFHE::nand_gate(a, b));
    GoldenFHE::Cipher nand_a_ab = boot.bootstrap(GoldenFHE::nand_gate(a, nand_ab));
    GoldenFHE::Cipher nand_b_ab = boot.bootstrap(GoldenFHE::nand_gate(b, nand_ab));
    GoldenFHE::Cipher a_xor_b = boot.bootstrap(GoldenFHE::nand_gate(nand_a_ab, nand_b_ab));
    
    // Re-bootstrap a_xor_b
    a_xor_b = boot.bootstrap(a_xor_b);
    
    // XOR(0,1)
    GoldenFHE::Cipher nand_ab2 = boot.bootstrap(GoldenFHE::nand_gate(a_xor_b, cin));
    GoldenFHE::Cipher nand_a_ab2 = boot.bootstrap(GoldenFHE::nand_gate(a_xor_b, nand_ab2));
    GoldenFHE::Cipher nand_b_ab2 = boot.bootstrap(GoldenFHE::nand_gate(cin, nand_ab2));
    
    // I-check ang nand_b_ab2 = NAND(cin, nand_ab2)
    NTL::ZZ_pX noise = nand_b_ab2.c0 + nand_b_ab2.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    std::cout << "nand_b_ab2: " << v << " (expect 0)\n";
    
    return 0;
}
