#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    // Test case (0,1,0): a=0, b=1, cin=0
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 2000);
    GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, false, 3000);
    
    // sum = a XOR b XOR cin = 0 XOR 1 XOR 0 = 1
    GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a, b);
    GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a_xor_b, cin);
    
    bool sum_result = GoldenFHE::decrypt(sum, sk);
    std::cout << "sum(0,1,0): " << sum_result << " (expected 1)\n";
    
    return 0;
}
