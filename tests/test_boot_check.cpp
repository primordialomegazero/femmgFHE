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
    
    // === XOR(1,1) ===
    GoldenFHE::Cipher nand_ab = GoldenFHE::nand_gate(a, b);
    nand_ab = boot.bootstrap(nand_ab);
    
    NTL::ZZ_pX noise1 = nand_ab.c0 + nand_ab.c1 * sk.sk;
    long v1 = NTL::conv<long>(NTL::coeff(noise1, 0));
    std::cout << "NAND(1,1): " << v1 << " (expect 0)\n";
    
    GoldenFHE::Cipher nand_a_ab = GoldenFHE::nand_gate(a, nand_ab);
    nand_a_ab = boot.bootstrap(nand_a_ab);
    
    NTL::ZZ_pX noise2 = nand_a_ab.c0 + nand_a_ab.c1 * sk.sk;
    long v2 = NTL::conv<long>(NTL::coeff(noise2, 0));
    std::cout << "NAND(1,0): " << v2 << " (expect " << golden_plain << ")\n";
    
    GoldenFHE::Cipher nand_b_ab = GoldenFHE::nand_gate(b, nand_ab);
    nand_b_ab = boot.bootstrap(nand_b_ab);
    
    NTL::ZZ_pX noise3 = nand_b_ab.c0 + nand_b_ab.c1 * sk.sk;
    long v3 = NTL::conv<long>(NTL::coeff(noise3, 0));
    std::cout << "NAND(1,0): " << v3 << " (expect " << golden_plain << ")\n";
    
    GoldenFHE::Cipher a_xor_b = GoldenFHE::nand_gate(nand_a_ab, nand_b_ab);
    a_xor_b = boot.bootstrap(a_xor_b);
    
    NTL::ZZ_pX noise4 = a_xor_b.c0 + a_xor_b.c1 * sk.sk;
    long v4 = NTL::conv<long>(NTL::coeff(noise4, 0));
    std::cout << "XOR(1,1): " << v4 << " (expect 0)\n";
    
    // === XOR(0,1) ===
    a_xor_b = boot.bootstrap(a_xor_b);  // fresh bootstrap
    
    GoldenFHE::Cipher nand_ab2 = GoldenFHE::nand_gate(a_xor_b, cin);
    nand_ab2 = boot.bootstrap(nand_ab2);
    
    NTL::ZZ_pX noise5 = nand_ab2.c0 + nand_ab2.c1 * sk.sk;
    long v5 = NTL::conv<long>(NTL::coeff(noise5, 0));
    std::cout << "NAND(0,1): " << v5 << " (expect " << golden_plain << ")\n";
    
    GoldenFHE::Cipher nand_a_ab2 = GoldenFHE::nand_gate(a_xor_b, nand_ab2);
    nand_a_ab2 = boot.bootstrap(nand_a_ab2);
    
    NTL::ZZ_pX noise6 = nand_a_ab2.c0 + nand_a_ab2.c1 * sk.sk;
    long v6 = NTL::conv<long>(NTL::coeff(noise6, 0));
    std::cout << "NAND(0,1): " << v6 << " (expect " << golden_plain << ")\n";
    
    GoldenFHE::Cipher nand_b_ab2 = GoldenFHE::nand_gate(cin, nand_ab2);
    nand_b_ab2 = boot.bootstrap(nand_b_ab2);
    
    NTL::ZZ_pX noise7 = nand_b_ab2.c0 + nand_b_ab2.c1 * sk.sk;
    long v7 = NTL::conv<long>(NTL::coeff(noise7, 0));
    std::cout << "NAND(1,1): " << v7 << " (expect 0)\n";
    
    GoldenFHE::Cipher sum = GoldenFHE::nand_gate(nand_a_ab2, nand_b_ab2);
    sum = boot.bootstrap(sum);
    
    NTL::ZZ_pX noise8 = sum.c0 + sum.c1 * sk.sk;
    long v8 = NTL::conv<long>(NTL::coeff(noise8, 0));
    std::cout << "XOR(0,1): " << v8 << " (expect " << golden_plain << ")\n";
    
    bool dec = GoldenFHE::decrypt(sum, sk);
    std::cout << "sum(1,1,1): " << dec << " (expect 1)\n";
    
    return 0;
}
