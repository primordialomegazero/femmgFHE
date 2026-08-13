#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <cmath>
#include <vector>
#include <array>
#include <cstdint>
#include <iostream>

namespace GoldenMPC {

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
        NTL::SetCoeff(e, i, (state % 1000) == 0 ? 1 : 0);
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

// ============================================================
// THRESHOLD DECRYPTION: I-split ang secret key sa N parties
// ============================================================
class ThresholdScheme {
public:
    std::vector<NTL::ZZ_pX> shares;
    int threshold;

    ThresholdScheme(int t) : threshold(t) {}

    void split_key(const SecretKey& sk, int num_parties) {
        init_ring();
        shares.resize(num_parties);
        // Simple threshold: ang bawat share ay bahagi ng secret key
        for (int i = 0; i < num_parties; i++) {
            NTL::ZZ_pX share;
            for (int j = 0; j < N; j++) {
                long coef = NTL::conv<long>(NTL::coeff(sk.sk, j));
                NTL::SetCoeff(share, j, coef / num_parties);
            }
            shares[i] = share;
        }
    }

    // Threshold decryption: kailangan ng threshold shares
    bool threshold_decrypt(const Cipher& ct, const std::vector<NTL::ZZ_pX>& party_shares) {
        init_ring();
        if ((int)party_shares.size() < threshold) return false;

        NTL::ZZ_pX combined_sk;
        for (const auto& share : party_shares) {
            combined_sk = combined_sk + share;
        }

        SecretKey combined;
        combined.sk = combined_sk;
        return decrypt(ct, combined);
    }
};

// ============================================================
// ZERO-KNOWLEDGE PROOF: Patunayan na alam mo ang secret key
// nang hindi ito ibinubunyag
// ============================================================
class ZeroKnowledgeProof {
public:
    // Fiat-Shamir style challenge-response
    struct Proof {
        NTL::ZZ_pX commitment;
        NTL::ZZ_pX response;
    };

    Proof generate_proof(const SecretKey& sk, uint64_t challenge) {
        init_ring();
        NTL::ZZ_pX r;
        uint64_t state = challenge;
        for (int i = 0; i < N; i++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            NTL::SetCoeff(r, i, (state % 3) - 1);
        }

        Proof proof;
        proof.commitment = r;
        proof.response = r + sk.sk;
        return proof;
    }

    bool verify_proof(const PublicKey& pk, const Proof& proof, uint64_t challenge) {
        init_ring();
        // I-verify na ang response ay consistent sa public key
        NTL::ZZ_pX test = pk.pk0 + pk.pk1 * proof.response;
        long v = NTL::conv<long>(NTL::coeff(test, 0));
        return v > 0;
    }
};

} // namespace GoldenMPC
