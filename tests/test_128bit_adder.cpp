#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <cstdint>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing 128-bit adder...\n";
    
    // Test cases: a + b (bilang uint64_t para sa lower 64 bits, upper 64 bits = 0)
    uint64_t test_cases[][2] = {
        {0, 0},
        {0, UINT64_MAX},
        {1, UINT64_MAX - 1},
        {123456789012345678, 987654321098765432},
        {UINT64_MAX, 0},
        {UINT64_MAX, 1},
        {UINT64_MAX, UINT64_MAX},  // = 2^65 - 2
        {0xFFFFFFFFFFFFFFFF, 0x123456789ABCDEF0}
    };
    
    int test_num = 0;
    for (auto& tc : test_cases) {
        uint64_t a_lo = tc[0];
        uint64_t b_lo = tc[1];
        uint64_t a_hi = 0, b_hi = 0;
        
        uint64_t base = 10000000000ULL + test_num * 1000000000ULL;
        
        // I-encrypt ang 128 bits (64 bits lower + 64 bits upper)
        GoldenFHE::Cipher a_bits[128], b_bits[128];
        for (int i = 0; i < 64; i++) {
            a_bits[i] = GoldenFHE::encrypt(pk, (a_lo >> i) & 1, base + i * 10000000ULL);
            b_bits[i] = GoldenFHE::encrypt(pk, (b_lo >> i) & 1, base + 1280000000ULL + i * 10000000ULL);
        }
        for (int i = 0; i < 64; i++) {
            a_bits[64 + i] = GoldenFHE::encrypt(pk, (a_hi >> i) & 1, base + 2560000000ULL + i * 10000000ULL);
            b_bits[64 + i] = GoldenFHE::encrypt(pk, (b_hi >> i) & 1, base + 3840000000ULL + i * 10000000ULL);
        }
        
        GoldenFHE::Cipher carry = GoldenFHE::encrypt(pk, false, base + 5120000000ULL);
        
        // Result bilang 128-bit value
        uint64_t result_lo = 0, result_hi = 0;
        
        for (int i = 0; i < 128; i++) {
            GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
            
            GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a_bits[i], b_bits[i]);
            GoldenFHE::Cipher sum_i = fhe.xor_chain(a_xor_b, carry);
            
            GoldenFHE::Cipher a_and_b = fhe.and_with_bootstrap(a_bits[i], b_bits[i]);
            GoldenFHE::Cipher carry_and_xor = fhe.and_with_bootstrap(carry, a_xor_b);
            carry = fhe.or_with_bootstrap(a_and_b, carry_and_xor);
            
            bool s = GoldenFHE::decrypt(sum_i, sk);
            if (s) {
                if (i < 64) result_lo |= (1ULL << i);
                else result_hi |= (1ULL << (i - 64));
            }
        }
        
        bool c = GoldenFHE::decrypt(carry, sk);
        
        // Expected result
        uint64_t expected_lo = a_lo + b_lo;
        uint64_t expected_hi = a_hi + b_hi + (c ? 1 : 0);
        if (expected_lo < a_lo) expected_hi++;  // carry from low
        
        std::cout << "Test " << test_num << ": ";
        std::cout << "a=" << a_hi << ":" << a_lo << " + ";
        std::cout << "b=" << b_hi << ":" << b_lo << " = ";
        std::cout << "result=" << result_hi << ":" << result_lo;
        std::cout << " (expected " << expected_hi << ":" << expected_lo << ")\n";
        
        if (result_lo != expected_lo || result_hi != expected_hi) {
            std::cout << "❌ FAILED\n";
            return 1;
        }
        
        test_num++;
    }
    
    std::cout << "\n✅ 128-BIT ADDER TEST PASSED (8/8)!\n";
    return 0;
}
