#include "../src/golden_privacy_system.h"
#include <iostream>
#include <set>

int main() {
    std::cout << "Nonce Values Check\n\n";
    
    GoldenPrivacySystem gps(42);
    
    // I-encrypt at i-trace ang nonces (indirectly sa c1)
    std::cout << "20 encryptions ng false (nonce=0 → Golden Angle):\n";
    
    std::set<long> c1_0_vals;
    std::set<long> c1_1_vals;
    
    for (int i = 0; i < 20; i++) {
        auto ct = gps.encrypt_data(false, 0);
        
        long c1_0 = NTL::conv<long>(NTL::coeff(ct.c1, 0));
        long c1_1 = NTL::conv<long>(NTL::coeff(ct.c1, 1));
        
        c1_0_vals.insert(c1_0);
        c1_1_vals.insert(c1_1);
        
        std::cout << "  ct[" << i << "]: c1[0]=" << c1_0 << " c1[1]=" << c1_1 << "\n";
    }
    
    std::cout << "\n";
    std::cout << "Unique c1[0]: " << c1_0_vals.size() << "\n";
    std::cout << "Unique c1[1]: " << c1_1_vals.size() << "\n\n";
    
    // Ang problema: ang GoldenFHE::encrypt ay gumagamit ng nonce para sa PRNG
    // Ang PRNG ay xorshift na may specific na state evolution
    // Kaya kahit magkaiba ang nonce, ang u polynomial ay maaaring mag-repeat
    
    // I-check ang xorshift output para sa Golden Angle nonces
    std::cout << "Xorshift analysis para sa Golden Angle nonces:\n";
    
    uint64_t counter = 0;
    double golden_angle = 2.0 * GP_PI / GP_PHI;
    
    for (int i = 0; i < 5; i++) {
        double val = std::fmod(counter * golden_angle, 2.0 * GP_PI);
        counter++;
        uint64_t nonce = static_cast<uint64_t>(val * (UINT64_MAX / (2.0 * GP_PI)));
        if (nonce == 0) nonce = 1;
        
        // I-simulate ang xorshift sa GoldenFHE::encrypt
        uint64_t state = nonce;
        std::cout << "  nonce[" << i << "]=" << nonce << " → ";
        for (int j = 0; j < 3; j++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            std::cout << state % 1000 << " ";
        }
        std::cout << "\n";
    }
    
    return 0;
}
