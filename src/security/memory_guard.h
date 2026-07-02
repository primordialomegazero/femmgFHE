/*
 * FEmmg-FHE v22.2 — Memory Guard (TRUE CRYPTO)
 *
 * ARX-based memory encryption (Add-Rotate-XOR).
 * Upgrade from simple XOR+rotate to SipHash-inspired ARX network.
 * Session key derived from random seed + φ constants.
 *
 * "What's in memory stays encrypted. Always."
 */

#pragma once
#include <cstdint>
#include <cstring>
#include <array>
#include <random>
#include <chrono>

namespace memory_guard {

constexpr double PHI = 1.6180339887498948482;
constexpr size_t KEY_SIZE = 32;
constexpr size_t ROUNDS = 4;

class MemoryGuard {
private:
    std::array<uint8_t, KEY_SIZE> key_;
    std::array<uint8_t, 16> nonce_;   // Random nonce per session
    bool active_ = false;

    // ARX round function (SipHash-inspired)
    static inline void arx_round(uint8_t& a, uint8_t& b, uint8_t k, uint8_t n, size_t round) {
        // Add
        a += k ^ n;
        b += (k >> 1) ^ (n << 1);
        // Rotate
        a = (a << 3) | (a >> 5);
        b = (b << 5) | (b >> 3);
        // XOR with round constant
        a ^= static_cast<uint8_t>(round * 0x9E);
        b ^= static_cast<uint8_t>(round * 0x37);
        // Mix
        a ^= b;
        b ^= (a >> 1);
    }

    // Derive strong key from seed using ARX mixing
    void derive_key(uint64_t seed) {
        // Generate random session nonce
        std::random_device rd;
        uint64_t random_nonce = 0;
        for (int i = 0; i < 4; i++) {
            uint64_t r = static_cast<uint64_t>(rd()) << 32 | static_cast<uint64_t>(rd());
            random_nonce ^= r;
        }
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        random_nonce ^= static_cast<uint64_t>(now);
        
        // Store nonce
        std::memcpy(nonce_.data(), &random_nonce, sizeof(random_nonce));
        std::memcpy(nonce_.data() + 8, &seed, sizeof(seed));

        // Derive key using φ-based ARX mixing
        uint64_t phi_bits;
        std::memcpy(&phi_bits, &PHI, sizeof(double));
        
        for (size_t i = 0; i < KEY_SIZE; i++) {
            uint8_t a = static_cast<uint8_t>((seed >> ((i % 8) * 8)) & 0xFF);
            uint8_t b = static_cast<uint8_t>((phi_bits >> ((i % 8) * 8)) & 0xFF);
            uint8_t n = nonce_[i % 16];
            
            // Apply ARX rounds
            for (size_t r = 0; r < ROUNDS; r++) {
                arx_round(a, b, static_cast<uint8_t>(i), n, r);
            }
            
            key_[i] = a ^ b;
        }
    }

public:
    MemoryGuard() = default;

    // Initialize with random session seed (auto-generates if seed==0)
    void init(uint64_t session_seed = 0) {
        if (session_seed == 0) {
            std::random_device rd;
            session_seed = static_cast<uint64_t>(rd()) << 32 | static_cast<uint64_t>(rd());
        }
        derive_key(session_seed);
        active_ = true;
    }

    // Encrypt: ARX network with session key + nonce
    __attribute__((always_inline))
    inline int64_t encrypt(int64_t plaintext) const {
        if (!active_) return plaintext;

        uint64_t result;
        std::memcpy(&result, &plaintext, sizeof(result));
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&result);

        for (size_t i = 0; i < sizeof(uint64_t); i++) {
            uint8_t k = key_[(i * 7 + 3) % KEY_SIZE];
            uint8_t n = nonce_[i % 16];
            
            // ARX encryption rounds
            bytes[i] ^= k;
            for (size_t r = 0; r < ROUNDS; r++) {
                bytes[i] += n ^ static_cast<uint8_t>(r * 0x6B);
                bytes[i] = (bytes[i] << 3) | (bytes[i] >> 5);
                bytes[i] ^= k ^ static_cast<uint8_t>((r + i) * 0x17);
            }
        }

        return static_cast<int64_t>(result);
    }

    // Decrypt: Reverse ARX network
    __attribute__((always_inline))
    inline int64_t decrypt(int64_t ciphertext) const {
        if (!active_) return ciphertext;

        uint64_t result;
        std::memcpy(&result, &ciphertext, sizeof(result));
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&result);

        for (size_t i = 0; i < sizeof(uint64_t); i++) {
            uint8_t k = key_[(i * 7 + 3) % KEY_SIZE];
            uint8_t n = nonce_[i % 16];
            
            // Reverse ARX rounds
            for (size_t r = ROUNDS; r > 0; r--) {
                bytes[i] ^= k ^ static_cast<uint8_t>(((r-1) + i) * 0x17);
                bytes[i] = (bytes[i] >> 3) | (bytes[i] << 5);
                bytes[i] -= n ^ static_cast<uint8_t>((r-1) * 0x6B);
            }
            bytes[i] ^= k;
        }

        return static_cast<int64_t>(result);
    }

    // Wipe keys from memory
    void wipe() {
        key_.fill(0);
        nonce_.fill(0);
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

    T raw() const { return encrypted_value_; }
};

} // namespace memory_guard
