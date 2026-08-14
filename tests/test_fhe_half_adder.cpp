#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk, 1);
    
    std::cout << "Testing half adder (a + b = sum, carry)...\n";
    
    bool test_cases[][2] = {
        {0, 0}, {0, 1}, {1, 0}, {1, 1}
    };
    
    for (auto& tc : test_cases) {
        bool a_bit = tc[0], b_bit = tc[1];
        
        GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, a_bit, 1000);
        GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, b_bit, 2000);
        
        // sum = a XOR b
        GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a, b);
        
        // carry = a AND b
        GoldenFHE::Cipher carry = fhe.and_with_bootstrap(a, b);
        
        bool sum_result = GoldenFHE::decrypt(sum, sk);
        bool carry_result = GoldenFHE::decrypt(carry, sk);
        
        bool expected_sum = a_bit ^ b_bit;
        bool expected_carry = a_bit & b_bit;
        
        std::cout << "a=" << a_bit << " b=" << b_bit 
                  << " | sum=" << sum_result << " (exp " << expected_sum << ")"
                  << " carry=" << carry_result << " (exp " << expected_carry << ")\n";
        
        if (sum_result != expected_sum || carry_result != expected_carry) {
            std::cout << "❌ FAILED\n";
            return 1;
        }
    }
    
    std::cout << "\n✅ Half adder test PASSED!\n";
    return 0;
}
