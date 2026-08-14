#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Test NAND(0,1) then bootstrap
    GoldenFHE::Cipher zero = GoldenFHE::encrypt(pk, false, 1000000);
    GoldenFHE::Cipher one = GoldenFHE::encrypt(pk, true, 1100000);
    
    GoldenFHE::Cipher nand01 = GoldenFHE::nand_gate(zero, one);
    NTL::ZZ_pX noise_before = nand01.c0 + nand01.c1 * sk.sk;
    long v_before = NTL::conv<long>(NTL::coeff(noise_before, 0));
    std::cout << "NAND(0,1) before bootstrap: " << v_before << "\n";
    
    GoldenFHE::Cipher nand01_boot = boot.bootstrap(nand01);
    NTL::ZZ_pX noise_after = nand01_boot.c0 + nand01_boot.c1 * sk.sk;
    long v_after = NTL::conv<long>(NTL::coeff(noise_after, 0));
    std::cout << "NAND(0,1) after bootstrap: " << v_after << "\n";
    
    // Ngayon NAND(nand01_boot, one) = NAND(1,1) dapat 0
    GoldenFHE::Cipher nand11 = GoldenFHE::nand_gate(nand01_boot, one);
    NTL::ZZ_pX noise11 = nand11.c0 + nand11.c1 * sk.sk;
    long v11 = NTL::conv<long>(NTL::coeff(noise11, 0));
    bool dec11 = GoldenFHE::decrypt(nand11, sk);
    std::cout << "NAND(bootstrapped_1, 1): " << v11 << " dec=" << dec11 << " (expect 0)\n";
    
    return 0;
}
