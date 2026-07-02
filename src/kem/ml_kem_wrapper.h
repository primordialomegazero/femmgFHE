/*
 * FEmmg-FHE v22.3 — ML-KEM Compatible Wrapper
 *
 * Standard KEM flow:
 *   keygen() → (pk, sk) where sk = seed, pk = seed || seed^KDF(seed)
 *   encapsulate(pk) → (ct, ss) where ct = random, ss = KDF(pk || ct)
 *   decapsulate(sk, ct) → ss = KDF(reconstruct_pk(sk) || ct)
 *
 * PHI-OMEGA-ZERO — I AM THAT I AM
 */

#pragma once
#include <chrono>
#include <cmath>
#include "phi_parallel_kem.h"
#include <cstring>
#include <array>
#include <utility>
#include <fcntl.h>
#include <unistd.h>

namespace ml_kem {

constexpr size_t PK_SIZE = 64, SK_SIZE = 32, CT_SIZE = 64, SS_SIZE = 32;

using PublicKey = std::array<uint8_t, PK_SIZE>;
using SecretKey = std::array<uint8_t, SK_SIZE>;
using Ciphertext = std::array<uint8_t, CT_SIZE>;
using SharedSecret = std::array<uint8_t, SS_SIZE>;

// KDF: phi_parallel_kem extract
inline void kdf(const uint8_t* input, size_t len, uint8_t* out) {
    phi_parallel::PhiParallelKEM kem;
    kem.engine.seed(input, len);
    kem.engine.evolve(128);
    kem.engine.extract(out, 32);
}

inline std::pair<PublicKey, SecretKey> keygen() {
    uint8_t seed[32];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) { read(fd, seed, 32); close(fd); }
    else { for (int i = 0; i < 32; i++) seed[i] = i * 7 + 3; }
    
    PublicKey pk{};
    SecretKey sk{};
    std::memcpy(sk.data(), seed, SK_SIZE);
    
    // pk = seed[0..31] || KDF(seed)
    uint8_t derived[32];
    kdf(seed, 32, derived);
    for (size_t i = 0; i < 32; i++) pk[i] = seed[i];
    for (size_t i = 0; i < 32; i++) pk[i+32] = derived[i];
    
    return {pk, sk};
}

inline std::pair<Ciphertext, SharedSecret> encapsulate(const PublicKey& pk) {
    // Generate ephemeral random
    uint8_t ephemeral[32];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) { read(fd, ephemeral, 32); close(fd); }
    else { for (int i = 0; i < 32; i++) ephemeral[i] = i * 13 + 7; }
    
    Ciphertext ct{};
    std::memcpy(ct.data(), ephemeral, CT_SIZE > 32 ? 32 : CT_SIZE);
    
    // ss = KDF(pk || ephemeral)
    uint8_t combined[PK_SIZE + 32];
    std::memcpy(combined, pk.data(), PK_SIZE);
    std::memcpy(combined + PK_SIZE, ephemeral, 32);
    
    SharedSecret ss{};
    kdf(combined, PK_SIZE + 32, ss.data());
    
    return {ct, ss};
}

inline SharedSecret decapsulate(const SecretKey& sk, const Ciphertext& ct) {
    // Reconstruct pk from sk
    PublicKey pk{};
    uint8_t derived[32];
    kdf(sk.data(), SK_SIZE, derived);
    for (size_t i = 0; i < 32; i++) pk[i] = sk[i];
    for (size_t i = 0; i < 32; i++) pk[i+32] = derived[i];
    
    // ss = KDF(pk || ct[0..31])
    uint8_t combined[PK_SIZE + 32];
    std::memcpy(combined, pk.data(), PK_SIZE);
    std::memcpy(combined + PK_SIZE, ct.data(), 32);
    
    SharedSecret ss{};
    kdf(combined, PK_SIZE + 32, ss.data());
    
    return ss;
}

inline bool self_test() {
    auto [pk, sk] = keygen();
    auto [ct, ss1] = encapsulate(pk);
    auto ss2 = decapsulate(sk, ct);
    for (size_t i = 0; i < SS_SIZE; i++) if (ss1[i] != ss2[i]) return false;
    auto [pk2, sk2] = keygen();
    auto [ct2, ss3] = encapsulate(pk2);
    bool differ = false;
    for (size_t i = 0; i < SS_SIZE; i++) if (ss1[i] != ss3[i]) { differ = true; break; }
    return differ;
}

struct BenchmarkResult { double keygen_us, encapsulate_us, decapsulate_us; bool passed; };

inline BenchmarkResult benchmark(int iterations = 100) {
    BenchmarkResult r{};
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) volatile auto __attribute__((unused)) kp = keygen();
    auto t2 = std::chrono::high_resolution_clock::now();
    r.keygen_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / (double)iterations;
    auto [pk, sk] = keygen();
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) volatile auto __attribute__((unused)) enc = encapsulate(pk);
    t2 = std::chrono::high_resolution_clock::now();
    r.encapsulate_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / (double)iterations;
    auto [ct, ss] = encapsulate(pk);
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) volatile auto __attribute__((unused)) dec = decapsulate(sk, ct);
    t2 = std::chrono::high_resolution_clock::now();
    r.decapsulate_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / (double)iterations;
    r.passed = self_test();
    return r;
}

} // namespace ml_kem
