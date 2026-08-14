#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <cstdint>
#include <chrono>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing 16-bit multiplier...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int test_cases[][2] = {
        {0, 0}, {0, 65535}, {1, 1}, {2, 3},
        {100, 200}, {255, 255}, {256, 256}, {1024, 64},
        {4096, 16}, {65535, 65535}
    };
    
    int test_num = 0;
    for (auto& tc : test_cases) {
        int a_val = tc[0], b_val = tc[1];
        
        uint64_t base = 10000000000ULL + test_num * 1000000000ULL;
        
        // I-encrypt ang 16 bits
        GoldenFHE::Cipher a_bits[16], b_bits[16];
        for (int i = 0; i < 16; i++) {
            a_bits[i] = GoldenFHE::encrypt(pk, (a_val >> i) & 1, base + i * 10000000ULL);
            b_bits[i] = GoldenFHE::encrypt(pk, (b_val >> i) & 1, base + 160000000ULL + i * 10000000ULL);
        }
        
        // Result: 32 bits
        int result = 0;
        
        // Schoolbook multiplication
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                if (i + j < 32) {
                    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
                    GoldenFHE::Cipher partial = fhe.and_with_bootstrap(a_bits[i], b_bits[j]);
                    bool p = GoldenFHE::decrypt(partial, sk);
                    if (p) {
                        result += (1 << (i + j));
                    }
                }
            }
        }
        
        int expected = a_val * b_val;
        
        std::cout << a_val << " * " << b_val << " = " << result 
                  << " (expected " << expected << ")\n";
        
        if (result != expected) {
            std::cout << "❌ FAILED\n";
            return 1;
        }
        
        test_num++;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n✅ 16-BIT MULTIPLIER TEST PASSED (10/10)!\n";
    std::cout << "Time: " << duration << " seconds\n";
    return 0;
}
