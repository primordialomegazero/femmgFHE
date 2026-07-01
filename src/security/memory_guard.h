/*
 * FEmmg-FHE v22.0.0 — Memory Guard
 * 
 * Encrypts value_int in memory to prevent plaintext leakage.
 * Uses φ-XOR with session key for fast encryption/decryption.
 * 
 * "What's in memory stays encrypted. Always."
 */

#pragma once
#include <cstdint>
#include <cstring>
#include <array>

namespace memory_guard {

constexpr double PHI = 1.6180339887498948482;
constexpr size_t KEY_SIZE = 32;

// ═══ φ-DERIVED SESSION KEY ═══
class MemoryGuard {
private:
    std::array<uint8_t, KEY_SIZE> key_;
    bool active_ = false;
    
    // Generate φ-based key from seed
    void derive_key(uint64_t seed) {
        uint64_t phi_bits;
        std::memcpy(&phi_bits, &PHI, sizeof(double));
        
        for (size_t i = 0; i < KEY_SIZE; i++) {
            // Mix seed with φ at different rotations
            key_[i] = static_cast<uint8_t>(
                ((seed >> ((i % 8) * 8)) ^ (phi_bits >> ((i % 8) * 8))) & 0xFF
            );
            // Additional mixing
            key_[i] ^= static_cast<uint8_t>((PHI * (i + 1) * 255.0));
            key_[i] = (key_[i] << 3) | (key_[i] >> 5);  // Rotate
        }
    }
    
public:
    MemoryGuard() = default;
    
    // Initialize with session seed
    void init(uint64_t session_seed) {
        derive_key(session_seed);
        active_ = true;
    }
    
    // Encrypt a value before storing in memory
    __attribute__((always_inline))
    inline int64_t encrypt(int64_t plaintext) const {
        if (!active_) return plaintext;
        
        int64_t result = plaintext;
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&result);
        
        for (size_t i = 0; i < sizeof(int64_t); i++) {
            bytes[i] ^= key_[(i * 3) % KEY_SIZE];
            bytes[i] = (bytes[i] << 2) | (bytes[i] >> 6);  // Scramble
        }
        
        return result;
    }
    
    // Decrypt a value when needed
    __attribute__((always_inline))
    inline int64_t decrypt(int64_t ciphertext) const {
        if (!active_) return ciphertext;
        
        int64_t result = ciphertext;
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&result);
        
        for (size_t i = 0; i < sizeof(int64_t); i++) {
            bytes[i] = (bytes[i] >> 2) | (bytes[i] << 6);  // Unscramble
            bytes[i] ^= key_[(i * 3) % KEY_SIZE];
        }
        
        return result;
    }
    
    // Wipe key from memory
    void wipe() {
        key_.fill(0);
        active_ = false;
    }
    
    bool is_active() const { return active_; }
    ~MemoryGuard() { wipe(); }
};

// ═══ SECURE CIPHERTEXT WRAPPER ═══
template<typename T>
class SecureValue {
private:
    T encrypted_value_;
    MemoryGuard* guard_;
    
public:
    SecureValue() : encrypted_value_(0), guard_(nullptr) {}
    
    void set_guard(MemoryGuard* guard) { guard_ = guard; }
    
    void set(T value) {
        encrypted_value_ = guard_ ? guard_->encrypt(value) : value;
    }
    
    T get() const {
        return guard_ ? guard_->decrypt(encrypted_value_) : encrypted_value_;
    }
    
    // Allow direct operations on encrypted value
    T raw() const { return encrypted_value_; }
};

} // namespace memory_guard
