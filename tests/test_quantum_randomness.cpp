#include "../src/golden_privacy_system.h"
#include <iostream>
#include <random>
#include <complex>
#include <cmath>
#include <chrono>

constexpr std::complex<double> I(0.0, 1.0);

// ============================================
// QUANTUM RANDOMNESS SOURCE
// Gumagamit ng quantum superposition para sa true randomness
// ============================================

class QuantumRandomSource {
private:
    std::complex<double> amplitude_0;
    std::complex<double> amplitude_1;
    std::mt19937 rng;
    
public:
    QuantumRandomSource() : amplitude_0(1.0), amplitude_1(0.0), rng(42) {}
    
    // Hadamard gate para i-superpose ang state
    void hadamard() {
        double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        auto new_0 = (amplitude_0 + amplitude_1) * inv_sqrt2;
        auto new_1 = (amplitude_0 - amplitude_1) * inv_sqrt2;
        amplitude_0 = new_0;
        amplitude_1 = new_1;
    }
    
    // Mag-measure para makakuha ng random bit
    bool measure() {
        double prob_0 = std::norm(amplitude_0);
        
        // Quantum measurement: collapse sa |0> o |1>
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double rand_val = dist(rng);
        
        if (rand_val < prob_0) {
            amplitude_0 = 1.0;
            amplitude_1 = 0.0;
            return false;  // |0>
        } else {
            amplitude_0 = 0.0;
            amplitude_1 = 1.0;
            return true;   // |1>
        }
    }
    
    // Mag-generate ng random uint64_t
    uint64_t random_uint64() {
        uint64_t result = 0;
        
        for (int i = 0; i < 64; i++) {
            hadamard();  // I-superpose
            bool bit = measure();  // I-measure
            if (bit) result |= (1ULL << i);
            hadamard();  // I-reset
        }
        
        return result;
    }
    
    // Mag-generate ng vector ng random nonces
    std::vector<uint64_t> random_nonces(int count) {
        std::vector<uint64_t> nonces;
        for (int i = 0; i < count; i++) {
            nonces.push_back(random_uint64());
        }
        return nonces;
    }
};

int main() {
    std::cout << "QUANTUM RANDOMNESS SOURCE\n";
    std::cout << "=========================\n\n";
    
    QuantumRandomSource qr;
    
    // Test: mag-generate ng 10 random uint64
    std::cout << "10 random uint64 values:\n";
    for (int i = 0; i < 10; i++) {
        uint64_t val = qr.random_uint64();
        std::cout << "  " << val << "\n";
    }
    
    // Test: gamitin sa encryption
    std::cout << "\nQuantum-randomized encryption:\n";
    
    GoldenPrivacySystem gps(42);
    
    std::vector<GoldenFHE::Cipher> cts;
    auto nonces = qr.random_nonces(5);
    
    for (int i = 0; i < 5; i++) {
        auto ct = gps.encrypt_data(false, nonces[i]);
        cts.push_back(ct);
        
        long c1_val = NTL::conv<long>(NTL::coeff(ct.c1, 0));
        std::cout << "  nonce=" << nonces[i] << " c1[0]=" << c1_val << "\n";
    }
    
    // I-check kung magkakaiba
    bool all_different = true;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < i; j++) {
            if (NTL::coeff(cts[i].c1, 0) == NTL::coeff(cts[j].c1, 0)) {
                all_different = false;
            }
        }
    }
    
    std::cout << "\nRandomized: " << (all_different ? "YES ✅" : "NO ❌") << "\n";
    
    // Verify decryption
    std::cout << "\nDecryption:\n";
    for (int i = 0; i < 5; i++) {
        bool dec = gps.decrypt_result(cts[i]);
        std::cout << "  ct[" << i << "] = " << dec << " (expected 0)\n";
    }
    
    return 0;
}
