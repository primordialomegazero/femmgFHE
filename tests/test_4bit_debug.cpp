#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    // Test 0 + 5 = 5 (binary: 0000 + 0101 = 0101)
    int a_val = 0, b_val = 5;
    
    uint64_t base = 1000000;
    
    GoldenFHE::Cipher a_bits[4], b_bits[4];
    for (int i = 0; i < 4; i++) {
        a_bits[i] = GoldenFHE::encrypt(pk, (a_val >> i) & 1, base + i * 100000);
        b_bits[i] = GoldenFHE::encrypt(pk, (b_val >> i) & 1, base + 400000 + i * 100000);
    }
    
    GoldenFHE::Cipher carry = GoldenFHE::encrypt(pk, false, base + 800000);
    
    for (int i = 0; i < 4; i++) {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        GoldenFHE::Cipher a_xor_b = fhe.xor_with_bootstrap(a_bits[i], b_bits[i]);
        GoldenFHE::Cipher sum_i = fhe.xor_chain(a_xor_b, carry);
        
        GoldenFHE::Cipher a_and_b = fhe.and_with_bootstrap(a_bits[i], b_bits[i]);
        GoldenFHE::Cipher carry_and_xor = fhe.and_with_bootstrap(carry, a_xor_b);
        carry = fhe.or_with_bootstrap(a_and_b, carry_and_xor);
        
        bool s = GoldenFHE::decrypt(sum_i, sk);
        bool c = GoldenFHE::decrypt(carry, sk);
        
        std::cout << "Bit " << i << ": a=" << ((a_val >> i) & 1) 
                  << " b=" << ((b_val >> i) & 1)
                  << " sum=" << s << " (expect " << (((a_val >> i) & 1) ^ ((b_val >> i) & 1)) << ")"
                  << " carry=" << c << "\n";
    }
    
    return 0;
}
