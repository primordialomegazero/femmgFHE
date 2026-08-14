#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <complex>
#include <cmath>
#include <array>

struct QuantumState {
    std::array<std::complex<double>, 4> amplitudes;
};

struct EncryptedQuantumState {
    GoldenFHE::Cipher amp_bits[4][2];
};

// I-encrypt ang quantum state - ang zero amplitude ay false
EncryptedQuantumState encrypt_quantum_state(const QuantumState& qs, 
                                             const GoldenFHE::PublicKey& pk) {
    EncryptedQuantumState eqs;
    
    for (int i = 0; i < 4; i++) {
        double magnitude = std::abs(qs.amplitudes[i]);
        bool has_amplitude = magnitude > 0.0001;  // Non-zero lang ang true
        
        bool real_sign = qs.amplitudes[i].real() >= 0;
        bool imag_sign = qs.amplitudes[i].imag() >= 0;
        
        // Mag-encode ng "amplitude exists" at sign
        eqs.amp_bits[i][0] = GoldenFHE::encrypt(pk, has_amplitude && real_sign, 1000000 + i * 1000);
        eqs.amp_bits[i][1] = GoldenFHE::encrypt(pk, has_amplitude && imag_sign, 1000000 + i * 1000 + 100);
    }
    
    return eqs;
}

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Quantum FHE Integration (v2)...\n";
    
    // Initial state: |00>
    QuantumState zero_state;
    zero_state.amplitudes = {1.0, 0.0, 0.0, 0.0};
    
    // I-encrypt
    EncryptedQuantumState encrypted = encrypt_quantum_state(zero_state, pk);
    
    // I-verify
    std::cout << "\nEncrypted state |00>:\n";
    for (int i = 0; i < 4; i++) {
        bool bit0 = GoldenFHE::decrypt(encrypted.amp_bits[i][0], sk);
        bool bit1 = GoldenFHE::decrypt(encrypted.amp_bits[i][1], sk);
        std::cout << "  |" << ((i >> 1) & 1) << ((i >> 0) & 1) << ">: " 
                  << (bit0 ? "1" : "0") << (bit1 ? "1" : "0") << "\n";
    }
    
    // Expected: |00> = 1, others = 0
    bool expected[] = {true, false, false, false};
    bool passed = true;
    for (int i = 0; i < 4; i++) {
        bool bit0 = GoldenFHE::decrypt(encrypted.amp_bits[i][0], sk);
        if (bit0 != expected[i]) {
            passed = false;
            break;
        }
    }
    
    if (passed) {
        std::cout << "\n✅ QUANTUM FHE INTEGRATION v2 PASSED!\n";
        return 0;
    } else {
        std::cout << "\n❌ FAILED\n";
        return 1;
    }
}
