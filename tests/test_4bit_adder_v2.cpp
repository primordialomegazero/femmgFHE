#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing 4-bit adder...\n";
    
    int test_cases[][2] = {
        {0, 0}, {0, 1}, {0, 5}, {0, 15},
        {1, 0}, {1, 1}, {1, 14}, {1, 15},
        {5, 5}, {5, 10}, {7, 8}, {10, 5},
        {15, 0}, {15, 1}, {15, 15}, {8, 7}
    };
    
    int test_num = 0;
    for (auto& tc : test_cases) {
        int a_val = tc[0], b_val = tc[1];
        
        // MAS MALAKING BASE para walang collision
        uint64_t base = 100000000 + test_num * 10000000;
        
        GoldenFHE::Cipher a_bits[4], b_bits[4];
        for (int i = 0; i < 4; i++) {
            a_bits[i] = GoldenFHE::encrypt(pk, (a_val >> i) & 1, base + i * 1000000);
            b_bits[i] = GoldenFHE::encrypt(pk, (b_val >> i) & 1, base + 4000000 + i * 1000000);
        }
        
        GoldenFHE::Cipher carry = GoldenFHE::encrypt(pk, false, base + 8000000);
        int result = 0;
        
        for (int i = 0; i < 4; i++) {
            GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
            
            GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a_bits[i], b_bits[i]);
            GoldenFHE::Cipher sum_i = fhe.xor_chain(a_xor_b, carry);
            
            GoldenFHE::Cipher a_and_b = fhe.and_with_bootstrap(a_bits[i], b_bits[i]);
            GoldenFHE::Cipher carry_and_xor = fhe.and_with_bootstrap(carry, a_xor_b);
            carry = fhe.or_with_bootstrap(a_and_b, carry_and_xor);
            
            bool s = GoldenFHE::decrypt(sum_i, sk);
            if (s) result |= (1 << i);
        }
        
        bool c = GoldenFHE::decrypt(carry, sk);
        if (c) result |= 16;
        
        int expected = a_val + b_val;
        
        std::cout << a_val << " + " << b_val << " = " << result 
                  << " (expected " << expected << ")\n";
        
        if (result != expected) {
            std::cout << "❌ FAILED\n";
            return 1;
        }
        
        test_num++;
    }
    
    std::cout << "\n✅ 4-BIT ADDER TEST PASSED (16/16)!\n";
    return 0;
}
