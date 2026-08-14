#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <vector>
#include <functional>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Programmatic Bootstrapping (v3 - pre-bootstrap input)...\n\n";
    
    // Noisy input = 1
    GoldenFHE::Cipher noisy = GoldenFHE::encrypt(pk, true, 70000000);
    for (int i = 0; i < 10; i++) {
        noisy.c0 = noisy.c0 + noisy.c0;
    }
    
    // STEP 1: I-bootstrap muna ang input para ma-reset ang noise
    GoldenBootstrapping::Bootstrapper boot(pk, sk);
    GoldenFHE::Cipher clean = boot.bootstrap(noisy);
    
    std::cout << "Before bootstrap: " << GoldenFHE::decrypt(noisy, sk) << "\n";
    std::cout << "After bootstrap: " << GoldenFHE::decrypt(clean, sk) << "\n\n";
    
    // STEP 2: I-evaluate ang LUT sa malinis na input
    GoldenFHE::Cipher lut0 = GoldenFHE::encrypt(pk, true, 10000000);   // NOT(0) = 1
    GoldenFHE::Cipher lut1 = GoldenFHE::encrypt(pk, false, 10000001);  // NOT(1) = 0
    
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    
    // Match entry 0
    GoldenFHE::Cipher expected0 = GoldenFHE::encrypt(pk, false, 40000000);
    GoldenFHE::Cipher eq0 = fhe.xor_with_bootstrap(clean, expected0);
    GoldenFHE::Cipher match0 = fhe.not_with_bootstrap(eq0);
    
    // Match entry 1
    GoldenFHE::Cipher expected1 = GoldenFHE::encrypt(pk, true, 40000001);
    GoldenFHE::Cipher eq1 = fhe.xor_with_bootstrap(clean, expected1);
    GoldenFHE::Cipher match1 = fhe.not_with_bootstrap(eq1);
    
    // Select
    GoldenFHE::Cipher sel0 = fhe.and_with_bootstrap(match0, lut0);
    GoldenFHE::Cipher sel1 = fhe.and_with_bootstrap(match1, lut1);
    GoldenFHE::Cipher result = fhe.or_with_bootstrap(sel0, sel1);
    
    bool output = GoldenFHE::decrypt(result, sk);
    
    std::cout << "NOT(1) after pre-bootstrap + LUT: " << output << " (expected 0)\n";
    
    if (output == false) {
        std::cout << "\n✅ PROGRAMMATIC BOOTSTRAPPING V3 PASSED!\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
