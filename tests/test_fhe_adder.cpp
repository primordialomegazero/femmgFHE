#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk, 2);
    
    std::cout << "Testing 1-bit full adder...\n";
    
    bool test_cases[][3] = {
        {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
        {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}
    };
    
    for (auto& tc : test_cases) {
        bool a_bit = tc[0], b_bit = tc[1], cin_bit = tc[2];
        
        GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, a_bit, 1000);
        GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, b_bit, 2000);
        GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, cin_bit, 3000);
        
        // sum = a XOR b XOR cin
        GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a, b);
        GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a_xor_b, cin);
        
        bool sum_result = GoldenFHE::decrypt(sum, sk);
        bool expected_sum = a_bit ^ b_bit ^ cin_bit;
        
        std::cout << "a=" << a_bit << " b=" << b_bit << " cin=" << cin_bit 
                  << " | sum=" << sum_result << " (expected " << expected_sum << ")\n";
        
        if (sum_result != expected_sum) {
            std::cout << "❌ FAILED\n";
            return 1;
        }
    }
    
    std::cout << "\n✅ 1-bit adder test passed!\n";
    return 0;
}
