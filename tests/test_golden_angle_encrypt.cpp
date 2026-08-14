#include "../src/golden_privacy_system.h"
#include <iostream>
#include <set>
#include <cmath>


// Golden Angle Random Generator - PERFECT distribution
class GoldenAngleRNG {
private:
    uint64_t counter;
    double golden_angle;
    
public:
    GoldenAngleRNG() : counter(0), golden_angle(2.0 * GP_PI / 1.6180339887498948482) {}
    
    uint64_t next() {
        double val = std::fmod(counter * golden_angle, 2.0 * GP_PI);
        counter++;
        
        // I-convert sa uint64
        uint64_t result = static_cast<uint64_t>(val * (UINT64_MAX / (2.0 * GP_PI)));
        return result;
    }
};

int main() {
    std::cout << "GOLDEN ANGLE ENCRYPTION TEST\n\n";
    
    GoldenPrivacySystem gps(42);
    GoldenAngleRNG ga_rng;
    
    // I-encrypt ang false nang 20 beses gamit ang Golden Angle nonce
    std::vector<GoldenFHE::Cipher> cts;
    for (int i = 0; i < 20; i++) {
        uint64_t nonce = ga_rng.next();
        cts.push_back(gps.encrypt_data(false, nonce));
    }
    
    // I-check uniqueness
    std::set<std::string> unique;
    for (const auto& ct : cts) {
        std::string key = std::to_string(NTL::conv<long>(NTL::coeff(ct.c0, 0))) + ":" +
                          std::to_string(NTL::conv<long>(NTL::coeff(ct.c0, 1))) + ":" +
                          std::to_string(NTL::conv<long>(NTL::coeff(ct.c1, 0))) + ":" +
                          std::to_string(NTL::conv<long>(NTL::coeff(ct.c1, 1)));
        unique.insert(key);
    }
    
    std::cout << "20 encryptions ng false (Golden Angle nonce):\n";
    std::cout << "  Unique ciphertexts: " << unique.size() << "/20\n";
    std::cout << "  Result: " << (unique.size() == 20 ? "PERFECT ✅" : "COLLISIONS ❌") << "\n\n";
    
    // Verify decryption
    bool all_correct = true;
    for (const auto& ct : cts) {
        if (gps.decrypt_result(ct) != false) all_correct = false;
    }
    
    std::cout << "  Decryption: " << (all_correct ? "ALL CORRECT ✅" : "FAILED ❌") << "\n";
    
    return 0;
}
