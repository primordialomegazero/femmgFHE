#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <cmath>
#include <vector>
#include <cstdint>

namespace GoldenLWEOpt {

constexpr int N = 1024;
constexpr long Q = 536870909;
constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

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
        NTL::SetCoeff(e, i, (state % 100) == 0 ? 1 : 0);
    }
    pk.pk0 = -(a * s + e);
    pk.pk1 = a;
}

struct Cipher {
    NTL::ZZ_pX c0, c1;
};

inline Cipher encrypt(const PublicKey& pk, bool bit, uint64_t nonce) {
    init_ring();
    NTL::ZZ_pX m;
    NTL::SetCoeff(m, 0, bit ? Q/2 : 0);

    uint64_t state = nonce;
    NTL::ZZ_pX u, e0, e1;
    for (int i = 0; i < N; i++) {
        state ^= (state << 13);
        state ^= (state >> 7);
        state ^= (state << 17);
        NTL::SetCoeff(u, i, (state % 3) - 1);
        NTL::SetCoeff(e0, i, (state % 100) == 0 ? 1 : 0);
        NTL::SetCoeff(e1, i, (state % 100) == 0 ? 1 : 0);
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
    return v > Q/8;
}

// Pansamantalang optimized NAND: decrypt-reencrypt pa rin pero may mas mabilis na PRNG
inline Cipher nand(const Cipher& a, const Cipher& b, const SecretKey& sk, const PublicKey& pk, uint64_t nonce) {
    bool ba = decrypt(a, sk);
    bool bb = decrypt(b, sk);
    bool bn = !(ba && bb);
    return encrypt(pk, bn, nonce);
}

} // namespace GoldenLWEOpt
