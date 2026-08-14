#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing 8-bit adder...\n";
    
    int test_cases[][2] = {
        {0, 0}, {0, 255}, {1, 254}, {100, 155},
        {128, 127}, {255, 0}, {255, 1}, {200, 55}
    };
    
    int test_num = 0;
    for (auto& tc : test_cases) {
        int a_val = tc[0], b_val = tc[1];
        
        uint64_t base = 1000000000 + test_num * 100000000;
        
        GoldenFHE::Cipher a_bits[8], b_bits[8];
        for (int i = 0; i < 8; i++) {
            a_bits[i] = GoldenFHE::encrypt(pk, (a_val >> i) & 1, base + i * 10000000);
            b_bits[i] = GoldenFHE::encrypt(pk, (b_val >> i) & 1, base + 80000000 + i * 10000000);
        }
        
        GoldenFHE::Cipher carry = GoldenFHE::encrypt(pk, false, base + 160000000);
        int result = 0;
        
        for (int i = 0; i < 8; i++) {
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
        if (c) result |= 256;
        
        int expected = a_val + b_val;
        
        std::cout << a_val << " + " << b_val << " = " << result 
                  << " (expected " << expected << ")\n";
        
        if (result != expected) {
            std::cout << "❌ FAILED\n";
            return 1;
        }
        
        test_num++;
    }
    
    std::cout << "\n✅ 8-BIT ADDER TEST PASSED (8/8)!\n";
    return 0;
}
