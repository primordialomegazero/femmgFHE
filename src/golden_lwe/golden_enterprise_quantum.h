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
#include <atomic>
#include <csignal>

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace GoldenEnterpriseQuantum {

constexpr int N = 1024;
constexpr long Q = 536870909;
constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr int MAX_DIM = 8;
constexpr int MAX_DEPTH = 4;
constexpr int TOTAL_DIMS = MAX_DIM * MAX_DEPTH;
constexpr int NUM_LAYERS = 4;

class Logger {
public:
    static void info(const std::string& msg) { std::cout << "[INFO] " << msg << "\n"; }
    static void error(const std::string& msg) { std::cerr << "[ERROR] " << msg << "\n"; }
    static void warn(const std::string& msg) { std::cout << "[WARN] " << msg << "\n"; }
};

static std::atomic<bool> g_running(true);
static void signal_handler(int sig) {
    Logger::warn("Shutting down gracefully...");
    g_running = false;
}
inline void install_signal_handlers() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
}
inline bool is_running() { return g_running; }

inline void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

class SecretKey {
public:
    NTL::ZZ_pX sk;
};

class PublicKey {
public:
    NTL::ZZ_pX pk0, pk1;
};

class EnterpriseKeys {
public:
    std::vector<PublicKey> pks;
    std::vector<SecretKey> sks;
};

inline void keygen(EnterpriseKeys& ek, uint64_t master_seed) {
    init_ring();
    ek.pks.resize(NUM_LAYERS);
    ek.sks.resize(NUM_LAYERS);

    uint64_t state = master_seed;
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        NTL::ZZ_pX s;
        for (int i = 0; i < N; i++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            long coef = (state % 3) - 1;
            NTL::SetCoeff(s, i, coef);
        }
        ek.sks[layer].sk = s;

        NTL::ZZ_pX a, e;
        for (int i = 0; i < N; i++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            NTL::SetCoeff(a, i, state % Q);
            NTL::SetCoeff(e, i, (state % 1000) == 0 ? 1 : 0);
        }
        ek.pks[layer].pk0 = -(a * s + e);
        ek.pks[layer].pk1 = a;
    }
    // Audit logging removed for performance
    (void)0;
}

struct Cipher {
    NTL::ZZ_pX c0, c1;
};

struct alignas(64) QuantumStateAligned {
    double values[TOTAL_DIMS];
};

struct QuantumCipher {
    Cipher classical;
    QuantumStateAligned quantum;
};

inline double swing(double v) { return -1.0 / v; }

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

// Dual-layer encryption na may quantum layer
inline QuantumCipher enterprise_encrypt(const EnterpriseKeys& ek, bool bit, uint64_t nonce) {
    init_ring();
    long golden_plain = static_cast<long>(Q / PHI);
    NTL::ZZ_pX msg;
    NTL::SetCoeff(msg, 0, bit ? golden_plain : 0);

    // Classical layer: 2-layer sequential
    Cipher current;
    current.c0 = msg;
    current.c1 = NTL::ZZ_pX();

    for (int layer = 0; layer < 2; layer++) {
        uint64_t state = nonce + layer * 100;
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            NTL::SetCoeff(u, i, (state % 3) - 1);
            NTL::SetCoeff(e0, i, (state % 1000) == 0 ? 1 : 0);
            NTL::SetCoeff(e1, i, (state % 1000) == 0 ? 1 : 0);
        }
        Cipher next;
        next.c0 = ek.pks[layer].pk0 * u + e0 + current.c0;
        next.c1 = ek.pks[layer].pk1 * u + e1;
        current = next;
    }

    QuantumCipher qc;
    qc.classical = current;

    // Quantum layer: SIMD fill na may golden ratio
    double base = bit ? PHI : PSI;
    simd_fill(qc.quantum.values, base);

    return qc;
}

inline bool enterprise_decrypt(const QuantumCipher& qc, const EnterpriseKeys& ek) {
    init_ring();

    // Classical decryption: 2 layers
    Cipher current = qc.classical;
    for (int layer = 1; layer >= 0; layer--) {
        NTL::ZZ_pX noise = current.c0 + current.c1 * ek.sks[layer].sk;
        current.c0 = noise;
        current.c1 = NTL::ZZ_pX();
    }

    long v = NTL::conv<long>(NTL::coeff(current.c0, 0));
    long threshold = static_cast<long>(Q / (2 * PHI));
    bool classical_bit = v > threshold;

    // Quantum verification
    int positives = 0;
    for (int i = 0; i < TOTAL_DIMS; i += 4) {
        if (qc.quantum.values[i] > 0) positives++;
        if (qc.quantum.values[i+1] > 0) positives++;
        if (qc.quantum.values[i+2] > 0) positives++;
        if (qc.quantum.values[i+3] > 0) positives++;
    }
    bool quantum_bit = positives > TOTAL_DIMS / 2;

    return classical_bit && quantum_bit;
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

// Batch API para sa enterprise
inline std::vector<QuantumCipher> batch_enterprise_encrypt(
    const EnterpriseKeys& ek,
    const std::vector<bool>& bits,
    uint64_t base_nonce
) {
    std::vector<QuantumCipher> results(bits.size());
    const int NUM_THREADS = 8;
    std::vector<std::thread> threads;

    auto worker = [&](int start, int end) {
        for (int i = start; i < end; i++) {
            results[i] = enterprise_encrypt(ek, bits[i], base_nonce + i);
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

} // namespace GoldenEnterpriseQuantum
