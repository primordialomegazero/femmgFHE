#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    std::cout << "Testing half adder with debug...\n";
    
    bool test_cases[][2] = {
        {0, 0}, {0, 1}, {1, 0}, {1, 1}
    };
    
    int test_num = 0;
    for (auto& tc : test_cases) {
        bool a_bit = tc[0], b_bit = tc[1];
        
        uint64_t nonce_a = 1000 + test_num * 100;
        uint64_t nonce_b = 2000 + test_num * 100;
        
        std::cout << "\nTest " << test_num << ": a=" << a_bit << " b=" << b_bit 
                  << " nonce_a=" << nonce_a << " nonce_b=" << nonce_b << "\n";
        
        GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, a_bit, nonce_a);
        GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, b_bit, nonce_b);
        
        GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a, b);
        GoldenFHE::Cipher carry = fhe.and_with_bootstrap(a, b);
        
        bool sum_result = GoldenFHE::decrypt(sum, sk);
        bool carry_result = GoldenFHE::decrypt(carry, sk);
        
        bool expected_sum = a_bit ^ b_bit;
        bool expected_carry = a_bit & b_bit;
        
        std::cout << "sum=" << sum_result << " (exp " << expected_sum << ")"
                  << " carry=" << carry_result << " (exp " << expected_carry << ")\n";
        
        test_num++;
    }
    
    return 0;
}
