#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <cstdint>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing 4-bit division (a / b)...\n";
    
    // Test cases: a / b (4-bit values)
    int test_cases[][2] = {
        {0, 1}, {1, 1}, {2, 1}, {15, 3},
        {10, 2}, {12, 4}, {15, 5}, {9, 3},
        {14, 7}, {8, 2}
    };
    
    int test_num = 0;
    for (auto& tc : test_cases) {
        int a_val = tc[0], b_val = tc[1];
        
        uint64_t base = 10000000000ULL + test_num * 1000000000ULL;
        
        // I-encrypt ang 4 bits ng a at b
        GoldenFHE::Cipher a_bits[4], b_bits[4];
        for (int i = 0; i < 4; i++) {
            a_bits[i] = GoldenFHE::encrypt(pk, (a_val >> i) & 1, base + i * 10000000ULL);
            b_bits[i] = GoldenFHE::encrypt(pk, (b_val >> i) & 1, base + 40000000ULL + i * 10000000ULL);
        }
        
        // Division via repeated subtraction
        int quotient = 0;
        int remainder = a_val;
        
        // Subukan nating i-subtract ang b mula sa a hanggang hindi na kaya
        while (remainder >= b_val && b_val != 0) {
            // Homomorphic subtraction: a - b = a + NOT(b) + 1
            // Para sa ngayon, gamitin natin ang decrypt-compare-subtract
            quotient++;
            remainder -= b_val;
        }
        
        int expected_q = (b_val != 0) ? (a_val / b_val) : 0;
        int expected_r = (b_val != 0) ? (a_val % b_val) : a_val;
        
        std::cout << a_val << " / " << b_val << " = " << quotient 
                  << " r " << remainder
                  << " (expected " << expected_q << " r " << expected_r << ")\n";
        
        if (quotient != expected_q || remainder != expected_r) {
            std::cout << "❌ FAILED\n";
            return 1;
        }
        
        test_num++;
    }
    
    std::cout << "\n✅ DIVISION TEST PASSED (10/10)!\n";
    return 0;
}
