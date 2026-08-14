#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Dalawang bootstrapped ciphertexts na parehong value 1
    GoldenFHE::Cipher one = GoldenFHE::encrypt(pk, true, 1000000);
    GoldenFHE::Cipher one_boot = boot.bootstrap(one);
    
    // NAND(bootstrapped_1, bootstrapped_1) = dapat 0
    GoldenFHE::Cipher nand_bb = GoldenFHE::nand_gate(one_boot, one_boot);
    
    NTL::ZZ_pX noise = nand_bb.c0 + nand_bb.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    bool dec = GoldenFHE::decrypt(nand_bb, sk);
    
    std::cout << "NAND(boot_1, boot_1): " << v << " dec=" << dec << " (expect 0)\n";
    
    // NAND(fresh_1, boot_1) = dapat 0
    GoldenFHE::Cipher one2 = GoldenFHE::encrypt(pk, true, 1100000);
    GoldenFHE::Cipher nand_fb = GoldenFHE::nand_gate(one2, one_boot);
    
    NTL::ZZ_pX noise2 = nand_fb.c0 + nand_fb.c1 * sk.sk;
    long v2 = NTL::conv<long>(NTL::coeff(noise2, 0));
    bool dec2 = GoldenFHE::decrypt(nand_fb, sk);
    
    std::cout << "NAND(fresh_1, boot_1): " << v2 << " dec=" << dec2 << " (expect 0)\n";
    
    return 0;
}
