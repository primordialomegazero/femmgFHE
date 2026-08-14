#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    // Test OR(0,0) = 0
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, false, 2000);
    
    GoldenFHE::Cipher or_result = fhe.or_with_bootstrap(a, b);
    
    bool dec = GoldenFHE::decrypt(or_result, sk);
    std::cout << "OR(0,0): " << dec << " (expected 0)\n";
    
    // Test OR(0,1) = 1
    GoldenFHE::Cipher b1 = GoldenFHE::encrypt(pk, true, 3000);
    GoldenFHE::Cipher or_result2 = fhe.or_with_bootstrap(a, b1);
    
    bool dec2 = GoldenFHE::decrypt(or_result2, sk);
    std::cout << "OR(0,1): " << dec2 << " (expected 1)\n";
    
    return 0;
}
