#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <vector>
#include <functional>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Programmatic Bootstrapping (FIXED)...\n\n";
    
    // NOT function
    auto not1 = [](const std::vector<bool>& inputs) {
        return !inputs[0];
    };
    
    // Setup LUT manually
    GoldenFHE::Cipher lut0 = GoldenFHE::encrypt(pk, true, 10000000);   // NOT(0) = 1
    GoldenFHE::Cipher lut1 = GoldenFHE::encrypt(pk, false, 10000001);  // NOT(1) = 0
    
    // Noisy input = 1
    GoldenFHE::Cipher input = GoldenFHE::encrypt(pk, true, 70000000);
    for (int i = 0; i < 10; i++) {
        input.c0 = input.c0 + input.c0;
    }
    
    // Manual LUT evaluation
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    // Match para sa entry 0
    GoldenFHE::Cipher expected0 = GoldenFHE::encrypt(pk, false, 40000000);
    GoldenFHE::Cipher eq0 = fhe.xor_with_bootstrap(input, expected0);
    GoldenFHE::Cipher match0 = fhe.not_with_bootstrap(eq0);
    
    // Match para sa entry 1
    GoldenFHE::Cipher expected1 = GoldenFHE::encrypt(pk, true, 40000001);
    GoldenFHE::Cipher eq1 = fhe.xor_with_bootstrap(input, expected1);
    GoldenFHE::Cipher match1 = fhe.not_with_bootstrap(eq1);
    
    // Select
    GoldenFHE::Cipher sel0 = fhe.and_with_bootstrap(match0, lut0);
    GoldenFHE::Cipher sel1 = fhe.and_with_bootstrap(match1, lut1);
    GoldenFHE::Cipher result = fhe.or_with_bootstrap(sel0, sel1);
    
    bool output = GoldenFHE::decrypt(result, sk);
    
    std::cout << "NOT(1) after refresh: " << output << " (expected 0)\n";
    
    if (output == false) {
        std::cout << "\n✅ FIXED VERSION PASSED!\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
