#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Test: Bootstrap ng NAND(0,1) = 1
    GoldenFHE::Cipher zero = GoldenFHE::encrypt(pk, false, 1000);
    GoldenFHE::Cipher one = GoldenFHE::encrypt(pk, true, 2000);
    GoldenFHE::Cipher nand01 = GoldenFHE::nand_gate(zero, one);
    
    NTL::ZZ_pX noise_before = nand01.c0 + nand01.c1 * sk.sk;
    long v_before = NTL::conv<long>(NTL::coeff(noise_before, 0));
    bool dec_before = GoldenFHE::decrypt(nand01, sk);
    
    std::cout << "BEFORE bootstrap:\n";
    std::cout << "  Value: " << v_before << "\n";
    std::cout << "  Decrypt: " << dec_before << " (expected 1)\n\n";
    
    // I-bootstrap
    GoldenFHE::Cipher bootstrapped = boot.bootstrap(nand01);
    
    NTL::ZZ_pX noise_after = bootstrapped.c0 + bootstrapped.c1 * sk.sk;
    long v_after = NTL::conv<long>(NTL::coeff(noise_after, 0));
    bool dec_after = GoldenFHE::decrypt(bootstrapped, sk);
    
    std::cout << "AFTER bootstrap:\n";
    std::cout << "  Value: " << v_after << " (expected ~" << golden_plain << ")\n";
    std::cout << "  Decrypt: " << dec_after << " (expected 1)\n";
    
    return 0;
}
