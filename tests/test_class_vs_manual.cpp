#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    // Test (1,0,1)
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, true, 10000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, false, 20000);
    GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, true, 30000);
    
    // Via class
    GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a, b);
    GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a_xor_b, cin);
    
    NTL::ZZ_pX noise = sum.c0 + sum.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    bool dec = GoldenFHE::decrypt(sum, sk);
    
    std::cout << "Class sum (1,0,1): " << v << " dec=" << dec << " (expect 0)\n";
    
    return 0;
}
