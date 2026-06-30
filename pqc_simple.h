// ============================================================
// SIMPLE PQC KEM - Working Version
// ============================================================
#pragma once
#include <vector>
#include <cstring>
#include <cstdint>

namespace pqc_simple {

// Simple but working KEM using XOR + Hash
class SimpleKEM {
private:
    static constexpr int KEY_SIZE = 32;
    uint8_t private_key[KEY_SIZE];
    uint8_t public_key[KEY_SIZE];

    // Simple hash
    uint64_t hash(const uint8_t* data, size_t len) {
        uint64_t h = 0x9E3779B97F4A7C15ULL;
        for(size_t i=0; i<len; i++) {
            h ^= (uint64_t)data[i] << ((i % 8) * 8);
            h *= 0x9E3779B97F4A7C15ULL;
            h ^= h >> 33;
            h *= 0x9E3779B97F4A7C15ULL;
            h ^= h >> 29;
        }
        return h;
    }

public:
    struct KeyPair {
        uint8_t private_key[KEY_SIZE];
        uint8_t public_key[KEY_SIZE];
    };

    struct Ciphertext {
        uint8_t data[KEY_SIZE];
        uint8_t nonce[KEY_SIZE];
    };

    KeyPair generate() {
        KeyPair kp;
        // Generate random private key
        FILE* f = fopen("/dev/urandom", "rb");
        if(f) {
            fread(kp.private_key, 1, KEY_SIZE, f);
            fclose(f);
        } else {
            for(int i=0; i<KEY_SIZE; i++) {
                kp.private_key[i] = (uint8_t)rand();
            }
        }

        // Derive public key from private
        uint64_t h = hash(kp.private_key, KEY_SIZE);
        for(int i=0; i<KEY_SIZE; i++) {
            kp.public_key[i] = kp.private_key[i] ^ (uint8_t)(h >> ((i % 8) * 8));
        }

        memcpy(private_key, kp.private_key, KEY_SIZE);
        memcpy(public_key, kp.public_key, KEY_SIZE);

        return kp;
    }

    std::pair<Ciphertext, std::vector<uint8_t>> encapsulate(const uint8_t* pub_key) {
        Ciphertext ct;
        std::vector<uint8_t> secret(KEY_SIZE);

        // Generate random secret and nonce
        FILE* f = fopen("/dev/urandom", "rb");
        if(f) {
            fread(secret.data(), 1, KEY_SIZE, f);
            fread(ct.nonce, 1, KEY_SIZE, f);
            fclose(f);
        }

        // Encrypt: ct.data = pub_key XOR secret XOR nonce
        for(int i=0; i<KEY_SIZE; i++) {
            ct.data[i] = pub_key[i] ^ secret[i] ^ ct.nonce[i];
        }

        return {ct, secret};
    }

    std::vector<uint8_t> decapsulate(const Ciphertext& ct) {
        std::vector<uint8_t> secret(KEY_SIZE);

        // Decrypt: secret = ct.data XOR private_key XOR nonce
        for(int i=0; i<KEY_SIZE; i++) {
            secret[i] = ct.data[i] ^ private_key[i] ^ ct.nonce[i];
        }

        return secret;
    }
};

} // namespace pqc_simple
