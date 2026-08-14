#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Dalawang magkaibang bootstrapped ciphertexts
    GoldenFHE::Cipher one = GoldenFHE::encrypt(pk, true, 1000000);
    GoldenFHE::Cipher one_boot1 = boot.bootstrap(one);
    GoldenFHE::Cipher one_boot2 = boot.bootstrap(one);
    
    // NAND(boot1, boot2) = dapat 0
    GoldenFHE::Cipher nand_bb = GoldenFHE::nand_gate(one_boot1, one_boot2);
    
    NTL::ZZ_pX noise = nand_bb.c0 + nand_bb.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    bool dec = GoldenFHE::decrypt(nand_bb, sk);
    
    std::cout << "NAND(boot1, boot2): " << v << " dec=" << dec << " (expect 0)\n";
    
    return 0;
}
