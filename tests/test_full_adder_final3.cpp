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
        bool a_bit = tc[0], b_bit = tc[1], cin_bit = tc[2];
        
        uint64_t base = 1000000 + test_num * 1000000;
        
        GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, a_bit, base);
        GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, b_bit, base + 100000);
        GoldenFHE::Cipher cin = GoldenFHE::encrypt(pk, cin_bit, base + 200000);
        
        // SUM: a XOR b XOR cin
        GoldenBootstrapping::UnlimitedFHE fhe_sum(pk, sk);
        GoldenFHE::Cipher a_xor_b = fhe_sum.xor_with_bootstrap(a, b);
        GoldenFHE::Cipher sum = fhe_sum.xor_chain(a_xor_b, cin);
        bool sum_result = GoldenFHE::decrypt(sum, sk);
        
        // COUT: (a AND b) OR (b AND cin) OR (a AND cin)
        GoldenBootstrapping::UnlimitedFHE fhe_cout(pk, sk);
        GoldenFHE::Cipher a_and_b = fhe_cout.and_with_bootstrap(a, b);
        GoldenFHE::Cipher b_and_cin = fhe_cout.and_with_bootstrap(b, cin);
        GoldenFHE::Cipher a_and_cin = fhe_cout.and_with_bootstrap(a, cin);
        GoldenFHE::Cipher or1 = fhe_cout.or_with_bootstrap(a_and_b, b_and_cin);
        GoldenFHE::Cipher cout = fhe_cout.or_with_bootstrap(or1, a_and_cin);
        bool cout_result = GoldenFHE::decrypt(cout, sk);
        
        bool expected_sum = a_bit ^ b_bit ^ cin_bit;
        bool expected_cout = (a_bit & b_bit) | (b_bit & cin_bit) | (a_bit & cin_bit);
        
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
