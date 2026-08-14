#include "../src/golden_privacy_system.h"
#include <iostream>
#include <random>

int main() {
    std::cout << "FIX: True Randomized Encryption\n\n";
    
    GoldenPrivacySystem gps(42);
    
    // I-encrypt ang same plaintext nang maraming beses
    // Dapat magkaiba ang ciphertexts dahil sa randomized u, e0, e1
    
    std::cout << "Encrypting false (0) 5 times with different nonces:\n";
    
    for (int i = 0; i < 5; i++) {
        auto ct = gps.encrypt_data(false, 97000 + i);
        long c0_val = NTL::conv<long>(NTL::coeff(ct.c0, 0));
        long c1_val = NTL::conv<long>(NTL::coeff(ct.c1, 0));
        
        std::cout << "  nonce=" << 97000 + i 
                  << " c0[0]=" << c0_val 
                  << " c1[0]=" << c1_val << "\n";
    }
    
    std::cout << "\n";
    
    // Ang problema: ang PRNG state ay derived from nonce
    // Kapag malapit ang nonces, magkatulad ang state evolution
    
    // Test: gamitin ang mas malayong nonces
    std::cout << "With far nonces:\n";
    for (int i = 0; i < 5; i++) {
        auto ct = gps.encrypt_data(false, 97000 + i * 1000);
        long c0_val = NTL::conv<long>(NTL::coeff(ct.c0, 0));
        
        std::cout << "  nonce=" << 97000 + i * 1000 
                  << " c0[0]=" << c0_val << "\n";
    }
    
    return 0;
}
