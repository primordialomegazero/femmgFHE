#include "../src/golden_privacy_system.h"
#include <iostream>
#include <set>
#include <sstream>

int main() {
    std::cout << "Full Ciphertext Uniqueness Check\n\n";
    
    GoldenPrivacySystem gps(42);
    
    // I-encrypt 20 beses at i-hash ang BUONG ciphertext
    std::set<std::string> unique_cts;
    
    for (int i = 0; i < 20; i++) {
        auto ct = gps.encrypt_data(false, 0);
        
        // I-hash ang buong polynomial
        std::stringstream ss;
        for (int j = 0; j < 10; j++) {  // First 10 coefficients
            ss << NTL::conv<long>(NTL::coeff(ct.c0, j)) << ":";
            ss << NTL::conv<long>(NTL::coeff(ct.c1, j)) << ":";
            ss << NTL::conv<long>(NTL::coeff(ct.c2, j)) << ":";
        }
        
        unique_cts.insert(ss.str());
    }
    
    std::cout << "20 encryptions (first 10 coefficients hashed):\n";
    std::cout << "  Unique ciphertexts: " << unique_cts.size() << "/20\n";
    std::cout << "  Result: " << (unique_cts.size() == 20 ? "PERFECT ✅" : "COLLISIONS ❌") << "\n";
    
    return 0;
}
