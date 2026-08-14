#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    
    // Test na may nonce 1200000
    GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, true, 1200000);
    GoldenFHE::Cipher zero = GoldenFHE::encrypt(pk, false, 1300000);
    
    GoldenFHE::Cipher nand01 = boot.bootstrap(GoldenFHE::nand_gate(zero, cin));
    GoldenFHE::Cipher nand11 = boot.bootstrap(GoldenFHE::nand_gate(cin, nand01));
    
    NTL::ZZ_pX noise = nand11.c0 + nand11.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    std::cout << "NAND(cin_1200000, nand01): " << v << " (expect 0)\n";
    
    // Test na may nonce 1000000
    GoldenFHE::Cipher cin2 = GoldenFHE::encrypt(pk, true, 1000000);
    GoldenFHE::Cipher zero2 = GoldenFHE::encrypt(pk, false, 1100000);
    
    GoldenFHE::Cipher nand01_2 = boot.bootstrap(GoldenFHE::nand_gate(zero2, cin2));
    GoldenFHE::Cipher nand11_2 = boot.bootstrap(GoldenFHE::nand_gate(cin2, nand01_2));
    
    NTL::ZZ_pX noise2 = nand11_2.c0 + nand11_2.c1 * sk.sk;
    long v2 = NTL::conv<long>(NTL::coeff(noise2, 0));
    std::cout << "NAND(cin_1000000, nand01): " << v2 << " (expect 0)\n";
    
    return 0;
}
