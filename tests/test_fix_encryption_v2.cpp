#include "../src/golden_privacy_system.h"
#include <iostream>

int main() {
    std::cout << "FIX V2: Proper Randomization Check\n\n";
    
    GoldenPrivacySystem gps(42);
    
    // I-encrypt ang false 5 beses
    // Dapat magkaiba ang c1 (randomized)
    
    std::cout << "Encrypting false with nonces 97000-97004:\n";
    
    std::vector<GoldenFHE::Cipher> cts;
    for (int i = 0; i < 5; i++) {
        cts.push_back(gps.encrypt_data(false, 97000 + i));
    }
    
    // I-check kung magkakaiba ang c1 values
    bool all_different = true;
    for (int i = 0; i < 5; i++) {
        long c1_val = NTL::conv<long>(NTL::coeff(cts[i].c1, 0));
        std::cout << "  ct[" << i << "] c1[0] = " << c1_val << "\n";
        
        for (int j = 0; j < i; j++) {
            if (NTL::coeff(cts[i].c1, 0) == NTL::coeff(cts[j].c1, 0)) {
                all_different = false;
            }
        }
    }
    
    std::cout << "\n";
    std::cout << "Ciphertexts ay randomized: " << (all_different ? "YES ✅" : "NO ❌") << "\n";
    
    // Mas magandang test: i-decrypt para i-verify correctness
    std::cout << "\nDecryption verification:\n";
    for (int i = 0; i < 5; i++) {
        bool dec = gps.decrypt_result(cts[i]);
        std::cout << "  ct[" << i << "] decrypts to: " << dec << " (expected 0)\n";
    }
    
    return 0;
}
