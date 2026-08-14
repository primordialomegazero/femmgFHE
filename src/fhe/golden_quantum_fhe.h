#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <cmath>
#include <vector>
#include <array>
#include <cstdint>
#include <iostream>

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace GoldenFHE {

constexpr int N = 1024;
constexpr long Q = 536870909;
constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr int MAX_DIM = 8;
constexpr int MAX_DEPTH = 4;
constexpr int TOTAL_DIMS = MAX_DIM * MAX_DEPTH;

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
        NTL::SetCoeff(e, i, (state % 10000) == 0 ? 1 : 0);
    }
    pk.pk0 = -(a * s + e);
    pk.pk1 = a;
}

// 3-component ciphertext para sa tamang multiplication
struct Cipher {
    NTL::ZZ_pX c0, c1, c2;
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
        NTL::SetCoeff(e0, i, (state % 10000) == 0 ? 1 : 0);
        NTL::SetCoeff(e1, i, (state % 10000) == 0 ? 1 : 0);
    }

    Cipher ct;
    ct.c0 = pk.pk0 * u + e0 + m;
    ct.c1 = pk.pk1 * u + e1;
    ct.c2 = NTL::ZZ_pX();  // Initialize to 0
    return ct;
}

inline bool decrypt(const Cipher& ct, const SecretKey& sk) {
    init_ring();
    NTL::ZZ_pX s = sk.sk;
    NTL::ZZ_pX s2 = s * s;
    
    NTL::ZZ_pX noise = ct.c0 + ct.c1 * s + ct.c2 * s2;
    long v = NTL::conv<long>(NTL::coeff(noise, 0));
    long threshold = static_cast<long>(Q / (2 * PHI));
    return v > threshold;
}

inline QuantumCipher quantum_encrypt(const PublicKey& pk, bool bit, uint64_t nonce) {
    QuantumCipher qc;
    qc.classical = encrypt(pk, bit, nonce);
    double base = bit ? PHI : PSI;
    simd_fill(qc.quantum.values, base);
    return qc;
}

inline bool quantum_decrypt(const QuantumCipher& qc, const SecretKey& sk) {
    bool classical_bit = decrypt(qc.classical, sk);
    int positives = 0;
    for (int i = 0; i < TOTAL_DIMS; i += 4) {
        if (qc.quantum.values[i] > 0) positives++;
        if (qc.quantum.values[i+1] > 0) positives++;
        if (qc.quantum.values[i+2] > 0) positives++;
        if (qc.quantum.values[i+3] > 0) positives++;
    }
    return classical_bit && (positives > TOTAL_DIMS / 2);
}

// TAMANG homomorphic multiplication na may 3 components
inline Cipher nand_gate(const Cipher& a, const Cipher& b) {
    init_ring();
    
    long golden_plain = static_cast<long>(Q / PHI);
    
    // Tamang multiplication: (a0 + a1*s + a2*s²) * (b0 + b1*s + b2*s²)
    NTL::ZZ_pX new_c0 = a.c0 * b.c0;
    NTL::ZZ_pX new_c1 = a.c0 * b.c1 + a.c1 * b.c0;
    NTL::ZZ_pX new_c2 = a.c0 * b.c2 + a.c1 * b.c1 + a.c2 * b.c0;
    
    // Rescale
    NTL::ZZ_p inv_golden;
    inv_golden = golden_plain;
    NTL::ZZ_p inv_val = NTL::inv(inv_golden);
    long inv_long = NTL::conv<long>(inv_val);
    
    new_c0 = new_c0 * inv_long;
    new_c1 = new_c1 * inv_long;
    new_c2 = new_c2 * inv_long;
    
    // NAND = golden_plain - scaled_result
    NTL::ZZ_pX golden_poly;
    NTL::SetCoeff(golden_poly, 0, golden_plain);
    
    Cipher r;
    r.c0 = golden_poly - new_c0;
    r.c1 = -new_c1;
    r.c2 = -new_c2;
    return r;
}

inline Cipher NOT(const Cipher& a) { return nand_gate(a, a); }

inline Cipher AND(const Cipher& a, const Cipher& b) {
    auto n = nand_gate(a, b);
    return nand_gate(n, n);
}

inline Cipher OR(const Cipher& a, const Cipher& b) {
    auto na = NOT(a);
    auto nb = NOT(b);
    return nand_gate(na, nb);
}

inline Cipher XOR(const Cipher& a, const Cipher& b) {
    auto n1 = nand_gate(a, b);
    auto n2 = nand_gate(a, n1);
    auto n3 = nand_gate(b, n1);
    return nand_gate(n2, n3);
}

} // namespace GoldenFHE
