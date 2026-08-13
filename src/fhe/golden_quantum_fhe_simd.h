#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <cmath>
#include <vector>
#include <array>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace GoldenQuantumFHESIMD {

constexpr int N = 1024;
constexpr long Q = 536870909;
constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr int MAX_DIM = 8;
constexpr int MAX_DEPTH = 4;
constexpr int TOTAL_DIMS = MAX_DIM * MAX_DEPTH; // 32

// Memory-aligned quantum state (32 doubles = 256 bytes = 4 cache lines)
struct alignas(64) QuantumStateAligned {
    double values[TOTAL_DIMS];
};

class SecretKey {
public:
    NTL::ZZ_pX sk;
};

class PublicKey {
public:
    NTL::ZZ_pX pk0, pk1;
};

inline void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

inline void keygen(PublicKey& pk, SecretKey& sk, uint64_t seed) {
    init_ring();
    NTL::ZZ_pX s;
    uint64_t state = seed;
    for (int i = 0; i < N; i++) {
        state ^= (state << 13);
        state ^= (state >> 7);
        state ^= (state << 17);
        long coef = (state % 3) - 1;
        NTL::SetCoeff(s, i, coef);
    }
    sk.sk = s;

    NTL::ZZ_pX a, e;
    for (int i = 0; i < N; i++) {
        state ^= (state << 13);
        state ^= (state >> 7);
        state ^= (state << 17);
        NTL::SetCoeff(a, i, state % Q);
        NTL::SetCoeff(e, i, (state % 1000) == 0 ? 1 : 0);
    }
    pk.pk0 = -(a * s + e);
    pk.pk1 = a;
}

struct Cipher {
    NTL::ZZ_pX c0, c1;
};

struct QuantumCipher {
    Cipher classical;
    QuantumStateAligned quantum;
};

inline double swing(double v) { return -1.0 / v; }

// SIMD fill: punan ang 32 doubles nang sabay-sabay
inline void simd_fill(double* dest, double value) {
#ifdef __AVX2__
    __m256d v = _mm256_set1_pd(value);
    for (int i = 0; i < TOTAL_DIMS; i += 4) {
        _mm256_store_pd(&dest[i], v);
    }
#else
    for (int i = 0; i < TOTAL_DIMS; i++) {
        dest[i] = value;
    }
#endif
}

inline Cipher encrypt(const PublicKey& pk, bool bit, uint64_t nonce) {
    init_ring();
    NTL::ZZ_pX m;
    long golden_plain = static_cast<long>(Q / PHI);
    NTL::SetCoeff(m, 0, bit ? golden_plain : 0);

    uint64_t state = nonce;
    NTL::ZZ_pX u, e0, e1;
    for (int i = 0; i < N; i++) {
        state ^= (state << 13);
        state ^= (state >> 7);
        state ^= (state << 17);
        NTL::SetCoeff(u, i, (state % 3) - 1);
        NTL::SetCoeff(e0, i, (state % 1000) == 0 ? 1 : 0);
        NTL::SetCoeff(e1, i, (state % 1000) == 0 ? 1 : 0);
    }

    Cipher ct;
    ct.c0 = pk.pk0 * u + e0 + m;
    ct.c1 = pk.pk1 * u + e1;
    return ct;
}

inline bool decrypt(const Cipher& ct, const SecretKey& sk) {
    init_ring();
    NTL::ZZ_pX noise = ct.c0 + ct.c1 * sk.sk;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    long threshold = static_cast<long>(Q / (2 * PHI));
    return v > threshold;
}

// SIMD-Optimized Quantum Encryption na may loop unrolling
inline QuantumCipher quantum_encrypt_simd(const PublicKey& pk, bool bit, uint64_t nonce) {
    QuantumCipher qc;
    qc.classical = encrypt(pk, bit, nonce);

    double base = bit ? PHI : PSI;
    // SIMD fill para sa 32 dimensions
    simd_fill(qc.quantum.values, base);

    return qc;
}

inline bool quantum_decrypt_simd(const QuantumCipher& qc, const SecretKey& sk) {
    bool classical_bit = decrypt(qc.classical, sk);

    int positives = 0;
    // Loop unrolling: 4 sa bawat iteration
    for (int i = 0; i < TOTAL_DIMS; i += 4) {
        if (qc.quantum.values[i] > 0) positives++;
        if (qc.quantum.values[i+1] > 0) positives++;
        if (qc.quantum.values[i+2] > 0) positives++;
        if (qc.quantum.values[i+3] > 0) positives++;
    }
    return classical_bit && (positives > TOTAL_DIMS / 2);
}

inline Cipher nand_gate(const Cipher& a, const Cipher& b) {
    init_ring();
    long golden_square = static_cast<long>((Q / PHI) * (1.0 / PHI));
    Cipher r;
    r.c0 = NTL::ZZ_pX();
    NTL::SetCoeff(r.c0, 0, golden_square);
    r.c0 = r.c0 - a.c0 * b.c0;
    r.c1 = NTL::ZZ_pX();
    return r;
}

// Batch API: 1000+ encryptions sabay-sabay
inline std::vector<QuantumCipher> batch_encrypt(const PublicKey& pk, const std::vector<bool>& bits, uint64_t base_nonce) {
    std::vector<QuantumCipher> results(bits.size());
    const int NUM_THREADS = 8;
    std::vector<std::thread> threads;
    std::mutex mtx;

    auto worker = [&](int start, int end) {
        for (int i = start; i < end; i++) {
            results[i] = quantum_encrypt_simd(pk, bits[i], base_nonce + i);
        }
    };

    int chunk = bits.size() / NUM_THREADS;
    for (int t = 0; t < NUM_THREADS; t++) {
        int start = t * chunk;
        int end = (t == NUM_THREADS - 1) ? bits.size() : start + chunk;
        threads.push_back(std::thread(worker, start, end));
    }

    for (auto& th : threads) {
        if (th.joinable()) th.join();
    }

    return results;
}

} // namespace GoldenQuantumFHESIMD
