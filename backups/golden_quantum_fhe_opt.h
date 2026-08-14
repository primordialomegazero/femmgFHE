#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <cmath>
#include <vector>
#include <array>
#include <cstdint>
#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>

namespace GoldenQuantumFHEOpt {

constexpr int N = 1024;
constexpr long Q = 536870909;
constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr int MAX_DIM = 8;
constexpr int MAX_DEPTH = 4;

inline void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

class SecretKey {
public:
    NTL::ZZ_pX sk;
};

class PublicKey {
public:
    NTL::ZZ_pX pk0, pk1;
};

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
    std::array<std::array<double, MAX_DIM>, MAX_DEPTH> quantum;
};

// Cache para sa swing operations
static std::vector<double> swing_cache;
static bool cache_initialized = false;

inline double swing(double v) {
    return -1.0 / v;
}

inline void init_swing_cache() {
    if (!cache_initialized) {
        swing_cache.resize(1024);
        double v = 1.0;
        for (int i = 0; i < 1024; i++) {
            swing_cache[i] = v;
            v = swing(v);
        }
        cache_initialized = true;
    }
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

// Parallel Quantum Encryption: 32 dimensions nang sabay-sabay
inline QuantumCipher quantum_encrypt_parallel(const PublicKey& pk, bool bit, uint64_t nonce) {
    init_swing_cache();
    QuantumCipher qc;
    qc.classical = encrypt(pk, bit, nonce);

    double base = bit ? PHI : PSI;
    const int TOTAL = MAX_DEPTH * MAX_DIM;
    const int NUM_THREADS = 4;
    std::vector<std::thread> threads;
    std::mutex mtx;

    auto worker = [&](int start, int end) {
        for (int idx = start; idx < end; idx++) {
            int d = idx / MAX_DIM;
            int dim = idx % MAX_DIM;
            // SIMD-like: direktang assignment gamit ang cache
            {
                std::lock_guard<std::mutex> lock(mtx);
                qc.quantum[d][dim] = base;
            }
        }
    };

    int chunk = TOTAL / NUM_THREADS;
    for (int t = 0; t < NUM_THREADS; t++) {
        int start = t * chunk;
        int end = (t == NUM_THREADS - 1) ? TOTAL : start + chunk;
        threads.push_back(std::thread(worker, start, end));
    }

    for (auto& th : threads) {
        if (th.joinable()) th.join();
    }

    return qc;
}

inline bool quantum_decrypt(const QuantumCipher& qc, const SecretKey& sk) {
    bool classical_bit = decrypt(qc.classical, sk);
    int positives = 0;
    int total = 0;
    for (int d = 0; d < MAX_DEPTH; d++) {
        for (int dim = 0; dim < MAX_DIM; dim++) {
            if (qc.quantum[d][dim] > 0) positives++;
            total++;
        }
    }
    return classical_bit && (positives > total / 2);
}

inline Cipher nand_gate(const Cipher& a, const Cipher& b) {
    init_ring();
    long golden_plain = static_cast<long>(Q / PHI);
    long golden_square = static_cast<long>((Q / PHI) * (1.0 / PHI));

    Cipher r;
    r.c0 = NTL::ZZ_pX();
    NTL::SetCoeff(r.c0, 0, golden_square);
    r.c0 = r.c0 - a.c0 * b.c0;
    r.c1 = NTL::ZZ_pX();
    return r;
}

} // namespace GoldenQuantumFHEOpt
