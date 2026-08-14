#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk, 1);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    // Test (0,1,1): a=0, b=1, cin=1, expected sum = 0
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 2000);
    GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, true, 3000);
    
    // a XOR b = 0 XOR 1 = 1
    GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a, b);
    NTL::ZZ_pX noise_axorb = a_xor_b.c0 + a_xor_b.c1 * sk.sk;
    long v_axorb = NTL::conv<long>(NTL::coeff(noise_axorb, 0));
    std::cout << "a XOR b: " << v_axorb << " (should be " << golden_plain << ")\n";
    
    // (a XOR b) XOR cin = 1 XOR 1 = 0
    GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a_xor_b, cin);
    NTL::ZZ_pX noise_sum = sum.c0 + sum.c1 * sk.sk;
    long v_sum = NTL::conv<long>(NTL::coeff(noise_sum, 0));
    std::cout << "sum: " << v_sum << " (should be 0, less than " << threshold << ")\n";
    
    return 0;
}
