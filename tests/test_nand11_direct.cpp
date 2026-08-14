#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Test NAND(1,1)
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, true, 1000000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 1100000);
    
    GoldenFHE::Cipher nand11 = GoldenFHE::nand_gate(a, b);
    
    NTL::ZZ_pX noise = nand11.c0 + nand11.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    bool dec = GoldenFHE::decrypt(nand11, sk);
    
    std::cout << "NAND(1,1): " << v << " dec=" << dec << " (expect 0)\n";
    
    return 0;
}
