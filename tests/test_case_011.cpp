#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    // Test (0,1,1): a=0, b=1, cin=1
    // expected_sum = 0^1^1 = 0
    // expected_cout = (0&1)|(1&1)|(0&1) = 0|1|0 = 1
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 1100000);
    GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, true, 1200000);
    
    GoldenBootstrapping::UnlimitedFHE fhe_sum(pk, sk);
    GoldenFHE::Cipher a_xor_b_sum = fhe_sum.xor_with_bootstrap(a, b);
    GoldenFHE::Cipher sum = fhe_sum.xor_with_bootstrap(a_xor_b_sum, cin);
    
    NTL::ZZ_pX noise = sum.c0 + sum.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    bool sum_result = GoldenFHE::decrypt(sum, sk);
    
    std::cout << "sum(0,1,1): " << v << " dec=" << sum_result << " (expect 0)\n";
    
    return 0;
}
