#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    // Test XOR(1,0)
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, true, 10000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, false, 20000);
    
    // Manual
    GoldenFHE::Cipher nand_ab = GoldenFHE::nand_gate(a, b);
    nand_ab = boot.bootstrap(nand_ab);
    GoldenFHE::Cipher nand_a_ab = GoldenFHE::nand_gate(a, nand_ab);
    nand_a_ab = boot.bootstrap(nand_a_ab);
    GoldenFHE::Cipher nand_b_ab = GoldenFHE::nand_gate(b, nand_ab);
    nand_b_ab = boot.bootstrap(nand_b_ab);
    GoldenFHE::Cipher manual_result = GoldenFHE::nand_gate(nand_a_ab, nand_b_ab);
    manual_result = boot.bootstrap(manual_result);
    
    // Via class
    GoldenFHE::Cipher class_result = fhe.xor_with_bootstrap(a, b);
    
    NTL::ZZ_pX noise_m = manual_result.c0 + manual_result.c1 * sk.sk;
    long vm = NTL::conv<long>(NTL::coeff(noise_m, 0));
    
    NTL::ZZ_pX noise_c = class_result.c0 + class_result.c1 * sk.sk;
    long vc = NTL::conv<long>(NTL::coeff(noise_c, 0));
    
    std::cout << "Manual XOR(1,0): " << vm << "\n";
    std::cout << "Class XOR(1,0): " << vc << "\n";
    std::cout << "Same: " << (vm == vc ? "YES" : "NO") << "\n";
    
    return 0;
}
