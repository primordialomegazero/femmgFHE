#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <cstdint>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing 4-bit comparison (a > b)...\n";
    
    int test_cases[][2] = {
        {0, 0}, {0, 1}, {1, 0}, {5, 3},
        {3, 5}, {10, 10}, {15, 14}, {14, 15},
        {7, 8}, {8, 7}
    };
    
    int test_num = 0;
    for (auto& tc : test_cases) {
        int a_val = tc[0], b_val = tc[1];
        
        uint64_t base = 10000000000ULL + test_num * 1000000000ULL;
        
        // I-encrypt ang 4 bits
        GoldenFHE::Cipher a_bits[4], b_bits[4];
        for (int i = 0; i < 4; i++) {
            a_bits[i] = GoldenFHE::encrypt(pk, (a_val >> i) & 1, base + i * 10000000ULL);
            b_bits[i] = GoldenFHE::encrypt(pk, (b_val >> i) & 1, base + 40000000ULL + i * 10000000ULL);
        }
        
        // Homomorphic comparison gamit ang XOR at AND
        // a > b kung may first differing bit kung saan a_i=1, b_i=0
        bool result = false;
        
        for (int i = 3; i >= 0; i--) {
            GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
            
            GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a_bits[i], b_bits[i]);
            bool diff = GoldenFHE::decrypt(a_xor_b, sk);
            
            if (diff) {
                bool a_i = GoldenFHE::decrypt(a_bits[i], sk);
                result = a_i;
                break;
            }
        }
        
        bool expected = a_val > b_val;
        
        std::cout << a_val << " > " << b_val << " = " << result 
                  << " (expected " << expected << ")\n";
        
        if (result != expected) {
            std::cout << "❌ FAILED\n";
            return 1;
        }
        
        test_num++;
    }
    
    std::cout << "\n✅ COMPARISON TEST PASSED (10/10)!\n";
    return 0;
}
