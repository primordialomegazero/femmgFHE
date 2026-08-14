#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    // Test XOR(0,1) in isolation
    {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000);
        GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 2000);
        
        GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a, b);
        bool sum_result = GoldenFHE::decrypt(sum, sk);
        
        std::cout << "XOR(0,1) isolated: " << sum_result << " (expected 1)\n";
    }
    
    // Test AND(0,1) in isolation
    {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000);
        GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 2000);
        
        GoldenFHE::Cipher carry = fhe.and_with_bootstrap(a, b);
        bool carry_result = GoldenFHE::decrypt(carry, sk);
        
        std::cout << "AND(0,1) isolated: " << carry_result << " (expected 0)\n";
    }
    
    // Test both in sequence (like the half adder)
    {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, false, 1000);
        GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, true, 2000);
        
        GoldenFHE::Cipher sum = fhe.xor_with_bootstrap(a, b);
        GoldenFHE::Cipher carry = fhe.and_with_bootstrap(a, b);
        
        bool sum_result = GoldenFHE::decrypt(sum, sk);
        bool carry_result = GoldenFHE::decrypt(carry, sk);
        
        std::cout << "XOR(0,1) in sequence: " << sum_result << " (expected 1)\n";
        std::cout << "AND(0,1) in sequence: " << carry_result << " (expected 0)\n";
    }
    
    return 0;
}
