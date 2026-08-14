#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <vector>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "LUT Debug: NOT function\n\n";
    
    // Setup LUT para sa NOT
    // lut[0] = NOT(false) = true
    // lut[1] = NOT(true) = false
    GoldenFHE::Cipher lut0 = GoldenFHE::encrypt(pk, true, 10000000);   // NOT(0) = 1
    GoldenFHE::Cipher lut1 = GoldenFHE::encrypt(pk, false, 10000001);  // NOT(1) = 0
    
    // Input: 1 (may noise)
    GoldenFHE::Cipher input = GoldenFHE::encrypt(pk, true, 70000000);
    
    // I-check kung paano nagma-match ang LUT
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    // Match para sa entry 0 (input=0)
    GoldenFHE::Cipher eq0 = fhe.xor_with_bootstrap(input, GoldenFHE::encrypt(pk, false, 40000000));
    GoldenFHE::Cipher not_equal0 = fhe.not_with_bootstrap(eq0);
    bool match0 = GoldenFHE::decrypt(not_equal0, sk);
    std::cout << "Match entry 0 (input=0): " << match0 << " (expected 0 kasi input=1)\n";
    
    // Match para sa entry 1 (input=1)
    GoldenFHE::Cipher eq1 = fhe.xor_with_bootstrap(input, GoldenFHE::encrypt(pk, true, 40000001));
    GoldenFHE::Cipher not_equal1 = fhe.not_with_bootstrap(eq1);
    bool match1 = GoldenFHE::decrypt(not_equal1, sk);
    std::cout << "Match entry 1 (input=1): " << match1 << " (expected 1 kasi input=1)\n";
    
    // Piliin ang LUT entry
    GoldenFHE::Cipher selected0 = fhe.and_with_bootstrap(not_equal0, lut0);
    GoldenFHE::Cipher selected1 = fhe.and_with_bootstrap(not_equal1, lut1);
    
    bool sel0 = GoldenFHE::decrypt(selected0, sk);
    bool sel1 = GoldenFHE::decrypt(selected1, sk);
    
    std::cout << "Selected lut[0]: " << sel0 << " (expected 0)\n";
    std::cout << "Selected lut[1]: " << sel1 << " (expected 0 kasi lut[1]=NOT(1)=0)\n";
    
    GoldenFHE::Cipher result = fhe.or_with_bootstrap(selected0, selected1);
    bool output = GoldenFHE::decrypt(result, sk);
    
    std::cout << "\nResult: " << output << " (expected 0)\n";
    
    return 0;
}
