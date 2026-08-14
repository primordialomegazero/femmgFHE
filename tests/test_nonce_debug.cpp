#include "../src/golden_privacy_system.h"
#include <iostream>
#include <set>

int main() {
    std::cout << "Quantum Random Nonce Debug\n\n";
    
    GoldenPrivacySystem gps(42);
    
    // I-trace kung ano ang nonce na na-generate
    std::cout << "Encrypting false with quantum nonce (nonce=0):\n";
    
    for (int i = 0; i < 10; i++) {
        auto ct = gps.encrypt_data(false, 0);
        
        long c0_0 = NTL::conv<long>(NTL::coeff(ct.c0, 0));
        long c0_1 = NTL::conv<long>(NTL::coeff(ct.c0, 1));
        long c1_0 = NTL::conv<long>(NTL::coeff(ct.c1, 0));
        long c1_1 = NTL::conv<long>(NTL::coeff(ct.c1, 1));
        
        std::cout << "  ct[" << i << "]: c0[0]=" << c0_0 
                  << " c0[1]=" << c0_1
                  << " c1[0]=" << c1_0
                  << " c1[1]=" << c1_1 << "\n";
    }
    
    std::cout << "\n";
    
    // Ang problema: quantum_random_nonce() ay tinatawag sa LOOB ng encrypt_data
    // Pero ang GoldenFHE::encrypt ay gumagamit ng nonce para sa PRNG
    // Kailangan nating i-check kung paano ginagamit ang nonce
    
    std::cout << "=== PRNG Analysis ===\n\n";
    
    // I-check ang PRNG state evolution para sa iba't ibang nonces
    for (uint64_t nonce : {1000000ULL, 1000001ULL, 1000002ULL}) {
        uint64_t state = nonce;
        std::cout << "  nonce=" << nonce << " → state evolution: ";
        for (int i = 0; i < 3; i++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            std::cout << state % 1000 << " ";
        }
        std::cout << "\n";
    }
    
    std::cout << "\n";
    
    // Ang issue: quantum_random_nonce() ay nag-ge-generate ng uint64
    // Pero ang encrypt_data ay nag-add ng 1000000 + nonce
    // Kapag ang nonce ay malaki, ang 1000000 + nonce ay maaaring mag-overflow
    // o mag-produce ng similar values
    
    std::cout << "=== Quantum Nonce Values ===\n\n";
    
    // I-extract ang quantum random nonces (indirectly)
    // I-encrypt at i-check ang pattern
    std::set<long> c1_0_values;
    for (int i = 0; i < 50; i++) {
        auto ct = gps.encrypt_data(false, 0);
        c1_0_values.insert(NTL::conv<long>(NTL::coeff(ct.c1, 0)));
    }
    
    std::cout << "Unique c1[0] values (50 encryptions): " << c1_0_values.size() << "\n";
    std::cout << "Expected: ~50 (kung truly random)\n";
    std::cout << "Observed: " << c1_0_values.size() << "\n\n";
    
    // I-print ang unang 10 unique values
    std::cout << "First 10 c1[0] values:\n";
    int count = 0;
    for (long v : c1_0_values) {
        std::cout << "  " << v << "\n";
        if (++count >= 10) break;
    }
    
    return 0;
}
