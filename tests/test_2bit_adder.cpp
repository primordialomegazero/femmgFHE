#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing 2-bit adder (a[1:0] + b[1:0])...\n";
    
    // Test: 0+0=0, 0+1=1, 0+2=2, 0+3=3, 1+0=1, 1+1=2, 1+2=3, 1+3=4
    int test_cases[][2] = {
        {0, 0}, {0, 1}, {0, 2}, {0, 3},
        {1, 0}, {1, 1}, {1, 2}, {1, 3},
        {2, 0}, {2, 1}, {2, 2}, {2, 3},
        {3, 0}, {3, 1}, {3, 2}, {3, 3}
    };
    
    int test_num = 0;
    for (auto& tc : test_cases) {
        int a_val = tc[0], b_val = tc[1];
        
        uint64_t base = 1000000 + test_num * 1000000;
        
        // I-encode ang 2-bit values
        GoldenFHE::Cipher a0 = GoldenFHE::encrypt(pk, a_val & 1, base);
        GoldenFHE::Cipher a1 = GoldenFHE::encrypt(pk, (a_val >> 1) & 1, base + 100000);
        GoldenFHE::Cipher b0 = GoldenFHE::encrypt(pk, b_val & 1, base + 200000);
        GoldenFHE::Cipher b1 = GoldenFHE::encrypt(pk, (b_val >> 1) & 1, base + 300000);
        
        // Bit 0: sum0 = a0 XOR b0, carry0 = a0 AND b0
        GoldenBootstrapping::UnlimitedFHE fhe0(pk, sk);
        GoldenFHE::Cipher sum0 = fhe0.xor_with_bootstrap(a0, b0);
        GoldenFHE::Cipher carry0 = fhe0.and_with_bootstrap(a0, b0);
        
        // Bit 1: sum1 = a1 XOR b1 XOR carry0, carry1 = (a1 AND b1) OR (carry0 AND (a1 XOR b1))
        GoldenBootstrapping::UnlimitedFHE fhe1(pk, sk);
        GoldenFHE::Cipher a1_xor_b1 = fhe1.xor_with_bootstrap(a1, b1);
        GoldenFHE::Cipher sum1 = fhe1.xor_chain(a1_xor_b1, carry0);
        GoldenFHE::Cipher a1_and_b1 = fhe1.and_with_bootstrap(a1, b1);
        GoldenFHE::Cipher carry0_and_xor = fhe1.and_with_bootstrap(carry0, a1_xor_b1);
        GoldenFHE::Cipher carry1 = fhe1.or_with_bootstrap(a1_and_b1, carry0_and_xor);
        
        bool s0 = GoldenFHE::decrypt(sum0, sk);
        bool s1 = GoldenFHE::decrypt(sum1, sk);
        bool c1 = GoldenFHE::decrypt(carry1, sk);
        
        int result = (s0 ? 1 : 0) + (s1 ? 2 : 0);
        int expected = a_val + b_val;
        
        std::cout << a_val << " + " << b_val << " = " << result 
                  << " (expected " << expected << ")\n";
        
        if (result != expected) {
            std::cout << "❌ FAILED\n";
            return 1;
        }
        
        test_num++;
    }
    
    std::cout << "\n✅ 2-BIT ADDER TEST PASSED (16/16)!\n";
    return 0;
}
