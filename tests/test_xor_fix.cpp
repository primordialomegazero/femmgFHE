#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

// Manual XOR function
GoldenFHE::Cipher fhe_xor(const GoldenFHE::Cipher& a, const GoldenFHE::Cipher& b, 
                          GoldenBootstrapping::Bootstrapper& boot, const GoldenFHE::SecretKey& sk) {
    GoldenFHE::Cipher nand_ab = GoldenFHE::nand_gate(a, b);
    nand_ab = boot.bootstrap(nand_ab);
    
    GoldenFHE::Cipher nand_a_ab = GoldenFHE::nand_gate(a, nand_ab);
    nand_a_ab = boot.bootstrap(nand_a_ab);
    
    GoldenFHE::Cipher nand_b_ab = GoldenFHE::nand_gate(b, nand_ab);
    nand_b_ab = boot.bootstrap(nand_b_ab);
    
    GoldenFHE::Cipher result = GoldenFHE::nand_gate(nand_a_ab, nand_b_ab);
    return boot.bootstrap(result);
}

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    
    // Test (0,1,1)
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 1100000);
    GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, true, 1200000);
    
    // XOR(0,1) = 1
    GoldenFHE::Cipher a_xor_b = fhe_xor(a, b, boot, sk);
    
    // I-bootstrap ang a_xor_b para fresh
    a_xor_b = boot.bootstrap(a_xor_b);
    
    // XOR(1,1) = 0
    GoldenFHE::Cipher sum = fhe_xor(a_xor_b, cin, boot, sk);
    
    NTL::ZZ_pX noise = sum.c0 + sum.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    bool dec = GoldenFHE::decrypt(sum, sk);
    
    std::cout << "sum(0,1,1): " << v << " dec=" << dec << " (expect 0)\n";
    
    return 0;
}
