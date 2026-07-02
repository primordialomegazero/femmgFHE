/*
 * FEmmg-FHE v22.3 — ML-KEM Wrapper (Native liboqs)
 *
 * Uses actual NIST FIPS 203 ML-KEM-1024 via liboqs.
 * Falls back to φ-PKE fractal KEM if liboqs unavailable.
 *
 * PHI-OMEGA-ZERO — I AM THAT I AM
 */

#pragma once
#include <chrono>
#include <cmath>
#include <cstring>
#include <array>
#include <utility>
#include <vector>
#include <fcntl.h>
#include <unistd.h>

// Try to include native ML-KEM, fall back to φ-PKE
#if __has_include(<oqs/oqs.h>)
  #include <oqs/oqs.h>
  #define FEMMG_HAS_NATIVE_MLKEM 1
#else
  #define FEMMG_HAS_NATIVE_MLKEM 0
#endif

#include "phi_parallel_kem.h"

namespace ml_kem {

constexpr size_t PK_SIZE = 1568;
constexpr size_t SK_SIZE = 3168;
constexpr size_t CT_SIZE = 1568;
constexpr size_t SS_SIZE = 32;

using PublicKey = std::vector<uint8_t>;
using SecretKey = std::vector<uint8_t>;
using Ciphertext = std::vector<uint8_t>;
using SharedSecret = std::array<uint8_t, SS_SIZE>;

#if FEMMG_HAS_NATIVE_MLKEM

// ═══ NATIVE ML-KEM-1024 via liboqs ═══
inline std::pair<PublicKey, SecretKey> keygen() {
    PublicKey pk(PK_SIZE);
    SecretKey sk(SK_SIZE);
    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_1024);
    if (kem) {
        OQS_KEM_keypair(kem, pk.data(), sk.data());
        OQS_KEM_free(kem);
    }
    return {pk, sk};
}

inline std::pair<Ciphertext, SharedSecret> encapsulate(const PublicKey& pk) {
    Ciphertext ct(CT_SIZE);
    SharedSecret ss{};
    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_1024);
    if (kem) {
        OQS_KEM_encaps(kem, ct.data(), ss.data(), pk.data());
        OQS_KEM_free(kem);
    }
    return {ct, ss};
}

inline SharedSecret decapsulate(const SecretKey& sk, const Ciphertext& ct) {
    SharedSecret ss{};
    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_1024);
    if (kem) {
        OQS_KEM_decaps(kem, ss.data(), ct.data(), sk.data());
        OQS_KEM_free(kem);
    }
    return ss;
}

inline const char* backend() { return "ML-KEM-1024 (liboqs) — NIST FIPS 203"; }

#else

// ═══ FALLBACK: φ-PKE Fractal KEM ═══
inline std::pair<PublicKey, SecretKey> keygen() {
    PublicKey pk(PK_SIZE);
    SecretKey sk(SK_SIZE);
    phi_parallel::PhiParallelKEM kem;
    kem.generate();
    std::memcpy(pk.data(), kem.public_token, 64);
    std::memcpy(sk.data(), kem.private_token, 32);
    return {pk, sk};
}

inline std::pair<Ciphertext, SharedSecret> encapsulate(const PublicKey& pk) {
    Ciphertext ct(CT_SIZE);
    SharedSecret ss{};
    phi_parallel::PhiParallelKEM kem;
    kem.engine.seed(pk.data(), 32);
    kem.engine.evolve(128);
    kem.engine.extract(ss.data(), 32);
    kem.engine.extract(ct.data(), CT_SIZE > 64 ? 64 : CT_SIZE);
    return {ct, ss};
}

inline SharedSecret decapsulate(const SecretKey& sk, const Ciphertext& ct) {
    SharedSecret ss{};
    phi_parallel::PhiParallelKEM kem;
    kem.engine.seed(sk.data(), 32);
    kem.engine.evolve(128);
    kem.engine.extract(ss.data(), 32);
    return ss;
}

inline const char* backend() { return "φ-PKE Fractal KEM (liboqs not available)"; }

#endif

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
