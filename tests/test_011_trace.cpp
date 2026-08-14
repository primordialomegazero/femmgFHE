#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    
    // Test (0,1,1)
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 1100000);
    GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, true, 1200000);
    
    // Step 1: a_xor_b = XOR(0,1) = 1
    GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a, b);
    NTL::ZZ_pX noise1 = a_xor_b.c0 + a_xor_b.c1 * sk.sk;
    long v1 = NTL::conv<long>(NTL::coeff(noise1, 0));
    bool d1 = GoldenFHE::decrypt(a_xor_b, sk);
    std::cout << "a_xor_b (0,1): " << v1 << " dec=" << d1 << " (expect 1)\n";
    
    // Step 2: sum = XOR(1,1) = 0
    GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a_xor_b, cin);
    NTL::ZZ_pX noise2 = sum.c0 + sum.c1 * sk.sk;
    long v2 = NTL::conv<long>(NTL::coeff(noise2, 0));
    bool d2 = GoldenFHE::decrypt(sum, sk);
    std::cout << "sum (1,1): " << v2 << " dec=" << d2 << " (expect 0)\n";
    
    return 0;
}
