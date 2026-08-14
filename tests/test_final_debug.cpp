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
    
    // XOR(1,1) = 0
    GoldenFHE::Cipher nand_ab = boot.bootstrap(GoldenFHE::nand_gate(a, b));  // 0
    GoldenFHE::Cipher nand_a_ab = boot.bootstrap(GoldenFHE::nand_gate(a, nand_ab));  // 1
    GoldenFHE::Cipher nand_b_ab = boot.bootstrap(GoldenFHE::nand_gate(b, nand_ab));  // 1
    GoldenFHE::Cipher a_xor_b = boot.bootstrap(GoldenFHE::nand_gate(nand_a_ab, nand_b_ab));  // 0
    
    a_xor_b = boot.bootstrap(a_xor_b);  // 0
    
    // XOR(0,1)
    GoldenFHE::Cipher nand_ab2 = boot.bootstrap(GoldenFHE::nand_gate(a_xor_b, cin));  // NAND(0,1)=1
    
    NTL::ZZ_pX noise_nand_ab2 = nand_ab2.c0 + nand_ab2.c1 * sk.sk;
    long v_nand_ab2 = NTL::conv<long>(NTL::coeff(noise_nand_ab2, 0));
    std::cout << "nand_ab2: " << v_nand_ab2 << " (expect " << golden_plain << ")\n";
    
    GoldenFHE::Cipher nand_a_ab2 = boot.bootstrap(GoldenFHE::nand_gate(a_xor_b, nand_ab2));  // NAND(0,1)=1
    
    NTL::ZZ_pX noise_nand_a_ab2 = nand_a_ab2.c0 + nand_a_ab2.c1 * sk.sk;
    long v_nand_a_ab2 = NTL::conv<long>(NTL::coeff(noise_nand_a_ab2, 0));
    std::cout << "nand_a_ab2: " << v_nand_a_ab2 << " (expect " << golden_plain << ")\n";
    
    GoldenFHE::Cipher nand_b_ab2 = boot.bootstrap(GoldenFHE::nand_gate(cin, nand_ab2));  // NAND(1,1)=0
    
    NTL::ZZ_pX noise_nand_b_ab2 = nand_b_ab2.c0 + nand_b_ab2.c1 * sk.sk;
    long v_nand_b_ab2 = NTL::conv<long>(NTL::coeff(noise_nand_b_ab2, 0));
    std::cout << "nand_b_ab2: " << v_nand_b_ab2 << " (expect 0)\n";
    
    GoldenFHE::Cipher sum = boot.bootstrap(GoldenFHE::nand_gate(nand_a_ab2, nand_b_ab2));  // NAND(1,0)=1
    
    NTL::ZZ_pX noise_sum = sum.c0 + sum.c1 * sk.sk;
    long v_sum = NTL::conv<long>(NTL::coeff(noise_sum, 0));
    std::cout << "sum: " << v_sum << " (expect " << golden_plain << ")\n";
    
    return 0;
}
