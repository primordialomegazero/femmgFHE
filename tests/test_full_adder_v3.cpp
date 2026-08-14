#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    std::cout << "Testing full adder (simplified cout)...\n";
    
    bool test_cases[][3] = {
        {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
        {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}
    };
    
    int test_num = 0;
    for (auto& tc : test_cases) {
        bool a_bit = tc[0], b_bit = tc[1], cin_bit = tc[2];
        
        uint64_t nonce_a = 1000 + test_num * 1000;
        uint64_t nonce_b = 2000 + test_num * 1000;
        uint64_t nonce_cin = 3000 + test_num * 1000;
        
        GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, a_bit, nonce_a);
        GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, b_bit, nonce_b);
        GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, cin_bit, nonce_cin);
        
        // sum = a XOR b XOR cin
        GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a, b);
        GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a_xor_b, cin);
        
        // cout = majority(a, b, cin) = (a AND b) OR (a AND cin) OR (b AND cin)
        // Para sa ngayon, gamitin natin: cout = a AND b (partial lang muna)
        GoldenFHE::Cipher cout = fhe.and_with_bootstrap(a, b);
        
        bool sum_result = GoldenFHE::decrypt(sum, sk);
        bool cout_result = GoldenFHE::decrypt(cout, sk);
        
        bool expected_sum = a_bit ^ b_bit ^ cin_bit;
        bool expected_cout = a_bit & b_bit;  // Simplified muna
        
        std::cout << "a=" << a_bit << " b=" << b_bit << " cin=" << cin_bit 
                  << " | sum=" << sum_result << " (exp " << expected_sum << ")"
                  << " cout(a&b)=" << cout_result << " (exp " << expected_cout << ")\n";
        
        test_num++;
    }
    
    return 0;
}
