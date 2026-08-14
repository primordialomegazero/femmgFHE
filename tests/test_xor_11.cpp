#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    // Test XOR(1,1) = 0
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, true, 10000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 20000);
    
    GoldenFHE::Cipher xor_result = fhe.xor_with_bootstrap(a, b);
    
    bool dec = GoldenFHE::decrypt(xor_result, sk);
    std::cout << "XOR(1,1): " << dec << " (expected 0)\n";
    
    return 0;
}
