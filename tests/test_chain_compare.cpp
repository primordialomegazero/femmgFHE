#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    
    // Test (1,0,1)
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, true, 10000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, false, 20000);
    GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, true, 30000);
    
    // XOR(1,0) = 1
    GoldenFHE::Cipher nand_ab = GoldenFHE::nand_gate(a, b);
    nand_ab = boot.bootstrap(nand_ab);
    GoldenFHE::Cipher nand_a_ab = GoldenFHE::nand_gate(a, nand_ab);
    nand_a_ab = boot.bootstrap(nand_a_ab);
    GoldenFHE::Cipher nand_b_ab = GoldenFHE::nand_gate(b, nand_ab);
    nand_b_ab = boot.bootstrap(nand_b_ab);
    GoldenFHE::Cipher a_xor_b = GoldenFHE::nand_gate(nand_a_ab, nand_b_ab);
    a_xor_b = boot.bootstrap(a_xor_b);
    
    NTL::ZZ_pX noise_axb = a_xor_b.c0 + a_xor_b.c1 * sk.sk;
    long v_axb = NTL::conv<long>(NTL::coeff(noise_axb, 0));
    std::cout << "a_xor_b (1,0): " << v_axb << " (expect 331804469)\n";
    
    // XOR(1, 1) = 0
    GoldenFHE::Cipher nand_ab2 = GoldenFHE::nand_gate(a_xor_b, cin);
    nand_ab2 = boot.bootstrap(nand_ab2);
    GoldenFHE::Cipher nand_a_ab2 = GoldenFHE::nand_gate(a_xor_b, nand_ab2);
    nand_a_ab2 = boot.bootstrap(nand_a_ab2);
    GoldenFHE::Cipher nand_b_ab2 = GoldenFHE::nand_gate(cin, nand_ab2);
    nand_b_ab2 = boot.bootstrap(nand_b_ab2);
    GoldenFHE::Cipher sum = GoldenFHE::nand_gate(nand_a_ab2, nand_b_ab2);
    sum = boot.bootstrap(sum);
    
    NTL::ZZ_pX noise_sum = sum.c0 + sum.c1 * sk.sk;
    long v_sum = NTL::conv<long>(NTL::coeff(noise_sum, 0));
    std::cout << "sum (1,0,1): " << v_sum << " (expect 0)\n";
    
    bool dec = GoldenFHE::decrypt(sum, sk);
    std::cout << "decrypt: " << dec << " (expect 0)\n";
    
    return 0;
}
