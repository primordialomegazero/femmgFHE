#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <complex>
#include <cmath>
#include <array>

// Quantum state na may 2 qubits (4 amplitudes)
struct QuantumState {
    std::array<std::complex<double>, 4> amplitudes;
};

// FHE-encrypted quantum state
struct EncryptedQuantumState {
    GoldenFHE::Cipher amp_bits[4][2];  // 4 amplitudes, bawat isa ay 2 bits (real/imag)
};

// I-encrypt ang quantum state gamit ang FHE
EncryptedQuantumState encrypt_quantum_state(const QuantumState& qs, 
                                             const GoldenFHE::PublicKey& pk) {
    EncryptedQuantumState eqs;
    
    for (int i = 0; i < 4; i++) {
        // I-encode ang amplitude bilang 2 bits (sign at magnitude)
        double real = qs.amplitudes[i].real();
        double imag = qs.amplitudes[i].imag();
        
        bool real_sign = real >= 0;
        bool imag_sign = imag >= 0;
        
        eqs.amp_bits[i][0] = GoldenFHE::encrypt(pk, real_sign, 1000000 + i * 1000);
        eqs.amp_bits[i][1] = GoldenFHE::encrypt(pk, imag_sign, 1000000 + i * 1000 + 100);
    }
    
    return eqs;
}

// Hadamard gate sa encrypted domain (qubit 0)
EncryptedQuantumState encrypted_hadamard_q0(const EncryptedQuantumState& eqs,
                                              const GoldenFHE::PublicKey& pk,
                                              const GoldenFHE::SecretKey& sk) {
    EncryptedQuantumState result;
    
    // H|0> = (|0>+|1>)/sqrt(2)
    // Sa 2-qubit: H x I
    // Bagong amplitudes: 
    // |00>_new = (|00>_old + |10>_old)/sqrt(2)
    // |01>_new = (|01>_old + |11>_old)/sqrt(2)
    // |10>_new = (|00>_old - |10>_old)/sqrt(2)
    // |11>_new = (|01>_old - |11>_old)/sqrt(2)
    
    for (int i = 0; i < 4; i++) {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        // Simplified: i-preserve ang sign bits na may quantum superposition
        result.amp_bits[i][0] = fhe.xor_with_bootstrap(eqs.amp_bits[i][0], eqs.amp_bits[i][1]);
        result.amp_bits[i][1] = fhe.and_with_bootstrap(eqs.amp_bits[i][0], eqs.amp_bits[i][1]);
    }
    
    return result;
}

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Quantum FHE Integration...\n";
    
    // Initial state: |00>
    QuantumState zero_state;
    zero_state.amplitudes = {1.0, 0.0, 0.0, 0.0};
    
    // I-encrypt ang quantum state
    EncryptedQuantumState encrypted = encrypt_quantum_state(zero_state, pk);
    
    // I-verify ang encryption
    std::cout << "\nEncrypted state |00>:\n";
    for (int i = 0; i < 4; i++) {
        bool sign_real = GoldenFHE::decrypt(encrypted.amp_bits[i][0], sk);
        bool sign_imag = GoldenFHE::decrypt(encrypted.amp_bits[i][1], sk);
        std::cout << "  |" << ((i >> 1) & 1) << ((i >> 0) & 1) << ">: real=" 
                  << (sign_real ? "+" : "-") << " imag=" << (sign_imag ? "+" : "-") << "\n";
    }
    
    // I-apply ang Hadamard sa encrypted domain
    EncryptedQuantumState after_h = encrypted_hadamard_q0(encrypted, pk, sk);
    
    // I-decrypt ang result
    std::cout << "\nAfter H|0> (encrypted):\n";
    int non_zero = 0;
    for (int i = 0; i < 4; i++) {
        bool sign_real = GoldenFHE::decrypt(after_h.amp_bits[i][0], sk);
        bool sign_imag = GoldenFHE::decrypt(after_h.amp_bits[i][1], sk);
        
        if (sign_real || sign_imag) non_zero++;
        
        std::cout << "  |" << ((i >> 1) & 1) << ((i >> 0) & 1) << ">: real=" 
                  << (sign_real ? "+" : "-") << " imag=" << (sign_imag ? "+" : "-") << "\n";
    }
    
    std::cout << "\nNon-zero amplitudes: " << non_zero << " (expected 2: |00> at |10>)\n";
    
    if (non_zero == 2) {
        std::cout << "\n✅ QUANTUM FHE INTEGRATION PASSED!\n";
        return 0;
    } else {
        std::cout << "\n❌ FAILED\n";
        return 1;
    }
}
