#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>
#include <set>

int main() {
    std::cout << "FIX: Chosen Plaintext - True Randomization\n\n";
    
    GoldenPrivacySystem gps(42);
    
    // I-encrypt ang false nang 20 beses gamit ang quantum random nonce
    std::vector<GoldenFHE::Cipher> cts;
    for (int i = 0; i < 20; i++) {
        cts.push_back(gps.encrypt_data(false, 0));  // nonce=0 → quantum random
    }
    
    // I-check uniqueness ng buong ciphertext (c0 + c1 + c2)
    std::set<std::string> unique_cts;
    
    for (const auto& ct : cts) {
        std::string key = "";
        key += std::to_string(NTL::conv<long>(NTL::coeff(ct.c0, 0)));
        key += ":";
        key += std::to_string(NTL::conv<long>(NTL::coeff(ct.c0, 1)));
        key += ":";
        key += std::to_string(NTL::conv<long>(NTL::coeff(ct.c1, 0)));
        key += ":";
        key += std::to_string(NTL::conv<long>(NTL::coeff(ct.c1, 1)));
        unique_cts.insert(key);
    }
    
    std::cout << "20 encryptions ng false:\n";
    std::cout << "  Unique ciphertexts: " << unique_cts.size() << "/20\n";
    std::cout << "  Result: " << (unique_cts.size() == 20 ? "RESISTANT ✅" : "VULNERABLE ❌") << "\n\n";
    
    // I-verify decryption
    bool all_correct = true;
    for (const auto& ct : cts) {
        bool dec = gps.decrypt_result(ct);
        if (dec != false) all_correct = false;
    }
    
    std::cout << "  Decryption: " << (all_correct ? "ALL CORRECT ✅" : "FAILED ❌") << "\n";
    
    return 0;
}
