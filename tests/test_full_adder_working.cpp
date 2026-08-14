#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing full adder...\n";
    
    bool test_cases[][3] = {
        {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
        {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}
    };
    
    int test_num = 0;
    for (auto& tc : test_cases) {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        bool a_bit = tc[0], b_bit = tc[1], cin_bit = tc[2];
        
        // Mas malaking nonce gaps para walang collision
        uint64_t nonce_a = 10000 + test_num * 10000;
        uint64_t nonce_b = 20000 + test_num * 10000;
        uint64_t nonce_cin = 30000 + test_num * 10000;
        
        GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, a_bit, nonce_a);
        GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, b_bit, nonce_b);
        GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, cin_bit, nonce_cin);
        
        // sum = a XOR b XOR cin
        GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a, b);
        GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a_xor_b, cin);
        
        // cout = (a AND b) OR (cin AND (a XOR b)) OR (a AND cin)
        GoldenFHE::Cipher a_and_b = fhe.and_with_bootstrap(a, b);
        GoldenFHE::Cipher cin_and_axorb = fhe.and_with_bootstrap(cin, a_xor_b);
        GoldenFHE::Cipher a_and_cin = fhe.and_with_bootstrap(a, cin);
        
        GoldenFHE::Cipher cout1 = fhe.xor_with_bootstrap(a_and_b, cin_and_axorb);
        GoldenFHE::Cipher cout = fhe.xor_with_bootstrap(cout1, a_and_cin);
        
        bool sum_result = GoldenFHE::decrypt(sum, sk);
        bool cout_result = GoldenFHE::decrypt(cout, sk);
        
        bool expected_sum = a_bit ^ b_bit ^ cin_bit;
        bool expected_cout = (a_bit & b_bit) | (cin_bit & (a_bit ^ b_bit));
        
        std::cout << "a=" << a_bit << " b=" << b_bit << " cin=" << cin_bit 
                  << " | sum=" << sum_result << " (exp " << expected_sum << ")"
                  << " cout=" << cout_result << " (exp " << expected_cout << ")\n";
        
        if (sum_result != expected_sum || cout_result != expected_cout) {
            std::cout << "❌ FAILED\n";
            return 1;
        }
        
        test_num++;
    }
    
    std::cout << "\n✅ FULL ADDER TEST PASSED!\n";
    return 0;
}
