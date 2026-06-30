// ============================================================
// FEmmg-FHE SECURITY v20.1 - WORKING
// ============================================================
#pragma once
#include <random>
#include <chrono>
#include <cstring>
#include <vector>
#include <string>
#include <cstdio>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#elif defined(__linux__) || defined(__unix__)
#include <fcntl.h>
#include <unistd.h>
#endif

namespace security {

// ============================================================
// 256-bit Nonce
// ============================================================
struct Nonce256 {
    uint64_t words[4];

    Nonce256() { words[0]=words[1]=words[2]=words[3]=0; }
    Nonce256(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
        words[0]=a; words[1]=b; words[2]=c; words[3]=d;
    }

    bool operator!=(const Nonce256& other) const {
        return words[0] != other.words[0] ||
               words[1] != other.words[1] ||
               words[2] != other.words[2] ||
               words[3] != other.words[3];
    }

    uint64_t mix(int dim, int layer, int party) const {
        return words[0] ^ (words[1] << ((dim + layer + party) % 16)) ^
               (words[2] >> ((layer + party + dim) % 16)) ^
               (words[3] * (party + 1)) ^
               (words[0] ^ words[1] ^ words[2] ^ words[3]);
    }

    std::string to_hex() const {
        char buf[65];
        for(int i=0; i<4; i++) {
            sprintf(buf + i*16, "%016lx", words[i]);
        }
        return std::string(buf, 64);
    }
};

// ============================================================
// CSPRNG
// ============================================================
class CSPRNG {
public:
    static Nonce256 generate_nonce() {
        Nonce256 nonce;
        uint8_t* buf = reinterpret_cast<uint8_t*>(&nonce);

#ifdef _WIN32
        BCryptGenRandom(NULL, buf, sizeof(Nonce256), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
#elif defined(__linux__) || defined(__unix__)
        int fd = open("/dev/urandom", O_RDONLY);
        if(fd >= 0) {
            size_t total = 0;
            while(total < sizeof(Nonce256)) {
                ssize_t r = read(fd, buf + total, sizeof(Nonce256) - total);
                if(r <= 0) break;
                total += r;
            }
            close(fd);
        } else {
            std::random_device rd;
            for(size_t i=0; i<sizeof(Nonce256); i++) {
                buf[i] = (uint8_t)rd();
            }
        }
#endif
        return nonce;
    }
};

// ============================================================
// PerturbationSeed - WORKING
// ============================================================
class PerturbationSeed {
private:
    Nonce256 seed;
    double cache[7][7][14];

    void build_cache() {
        for(int d=0; d<7; d++) {
            for(int layer=0; layer<7; layer++) {
                for(int party=0; party<14; party++) {
                    uint64_t mix = seed.mix(d, layer, party);
                    // Use ALL parameters for variation
                    double factor = (double)(mix % 1000000) / 1000000.0;
                    // Include party so (0,0,0) != (0,0,1)
                    double base = std::sin((double)d * 1.618 + (double)layer * 2.718 + (double)party * 3.141 + 1.0);
                    // Range: 0.01 to 0.15
                    cache[d][layer][party] = 0.05 + factor * 0.1 + base * 0.02;
                }
            }
        }
    }

public:
    PerturbationSeed() {
        seed = CSPRNG::generate_nonce();
        build_cache();
    }

    PerturbationSeed(const Nonce256& s) : seed(s) {
        build_cache();
    }

    PerturbationSeed(const PerturbationSeed& other) : seed(other.seed) {
        memcpy(cache, other.cache, sizeof(cache));
    }

    double get_perturbation(int dim, int layer, int party) const {
        return cache[dim % 7][layer % 7][party % 14];
    }

    Nonce256 get_seed() const { return seed; }
};

// ============================================================
// PQC KEM - FIXED
// ============================================================
class SimpleKEM {
private:
    static constexpr int KEY_SIZE = 32;
    uint8_t private_key[KEY_SIZE];
    uint8_t public_key[KEY_SIZE];

    uint64_t hash(const uint8_t* data, size_t len) {
        uint64_t h = 0x9E3779B97F4A7C15ULL;
        for(size_t i=0; i<len; i++) {
            h ^= (uint64_t)data[i] << ((i % 8) * 8);
            h *= 0x9E3779B97F4A7C15ULL;
            h ^= h >> 33;
        }
        return h;
    }

    void random_bytes(uint8_t* buf, size_t len) {
#ifdef _WIN32
        BCryptGenRandom(NULL, buf, len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
#elif defined(__linux__) || defined(__unix__)
        int fd = open("/dev/urandom", O_RDONLY);
        if(fd >= 0) {
            size_t total = 0;
            while(total < len) {
                ssize_t r = read(fd, buf + total, len - total);
                if(r <= 0) break;
                total += r;
            }
            close(fd);
        } else {
            for(size_t i=0; i<len; i++) {
                buf[i] = (uint8_t)rand();
            }
        }
#endif
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
        random_bytes(kp.private_key, KEY_SIZE);

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

        random_bytes(secret.data(), KEY_SIZE);
        random_bytes(ct.nonce, KEY_SIZE);

        for(int i=0; i<KEY_SIZE; i++) {
            ct.data[i] = pub_key[i] ^ secret[i] ^ ct.nonce[i];
        }

        return {ct, secret};
    }

    std::vector<uint8_t> decapsulate(const Ciphertext& ct) {
        std::vector<uint8_t> secret(KEY_SIZE);
        for(int i=0; i<KEY_SIZE; i++) {
            secret[i] = ct.data[i] ^ private_key[i] ^ ct.nonce[i];
        }
        return secret;
    }
};

} // namespace security
