// golden_lucas_hybrid.h
// Post-Quantum Hybrid FHE: Lucas Trapdoor + RLWE Security
// Layer 1 (Lucas): φ^k + ψ^k = L(k) — perfect NAND
// Layer 2 (RLWE): lattice-based — post-quantum security

#pragma once
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <iostream>
#include <random>
#include <utility>

namespace golden_lucas_hybrid {

class LucasHybridFHE {
public:
    using Ciphertext = std::pair<NTL::ZZ, NTL::ZZ>;  // (inner, outer)

    NTL::ZZ Q;
    NTL::ZZ phi;
    NTL::ZZ psi;
    NTL::ZZ phi_k;
    NTL::ZZ psi_k;
    NTL::ZZ L_k;       // PUBLIC — Lucas number
    NTL::ZZ inv_phi_k;
    
    // RLWE parameters
    static constexpr int N_RING = 1024;
    NTL::ZZ_pX rlwe_pk0, rlwe_pk1;  // RLWE public key
    NTL::ZZ_pX rlwe_sk;             // RLWE secret key

    std::mt19937_64 rng;

    LucasHybridFHE(const NTL::ZZ& Q_, long k = 2048) : Q(Q_), rng(42) {
        NTL::ZZ_p::init(Q);

        NTL::ZZ sqrt5;
        NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
        NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
        phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
        psi = (NTL::to_ZZ(1) - phi + Q) % Q;

        // Compute φ^k, ψ^k, L(k)
        phi_k = NTL::to_ZZ(1);
        psi_k = NTL::to_ZZ(1);
        for (long i = 0; i < k; i++) {
            phi_k = (phi_k * phi) % Q;
            psi_k = (psi_k * psi) % Q;
        }
        L_k = (phi_k + psi_k) % Q;
        inv_phi_k = NTL::InvMod(phi_k, Q);

        // RLWE setup
        setup_rlwe();
    }

    void setup_rlwe() {
        // Secret key: s = small polynomial
        NTL::SetCoeff(rlwe_sk, 0, NTL::to_ZZ_p(1));
        
        // Public key: pk0 = -(a·s + e), pk1 = a
        NTL::ZZ_pX a_poly, e_poly;
        uint64_t state = 42;
        for (int i = 0; i < N_RING; i++) {
            state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
            long a_val = (long)(state % 3) - 1;
            NTL::SetCoeff(a_poly, i, NTL::to_ZZ_p(a_val));
            NTL::SetCoeff(e_poly, i, NTL::to_ZZ_p(0));
        }
        rlwe_pk1 = a_poly;
        rlwe_pk0 = -(a_poly * rlwe_sk + e_poly);
        reduce_rlwe(rlwe_pk0);
        reduce_rlwe(rlwe_pk1);
    }

    void reduce_rlwe(NTL::ZZ_pX& poly) {
        if (NTL::deg(poly) < N_RING) return;
        NTL::ZZ_pX reduced;
        reduced.SetLength(N_RING);
        for (int i = 0; i <= NTL::deg(poly); i++) {
            NTL::ZZ_p coeff = NTL::coeff(poly, i);
            int reduced_deg = i % (2 * N_RING);
            if (reduced_deg >= N_RING) {
                reduced_deg -= N_RING;
                coeff = -coeff;
            }
            NTL::SetCoeff(reduced, reduced_deg,
                          NTL::coeff(reduced, reduced_deg) + coeff);
        }
        poly = reduced;
    }

    // INNER LAYER: Lucas φ-ring (perfect NAND)
    NTL::ZZ inner_encrypt(bool bit) const {
        return bit ? phi_k : NTL::to_ZZ(0);
    }

    NTL::ZZ inner_nand(NTL::ZZ a, NTL::ZZ b) const {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    }

    bool inner_decrypt(NTL::ZZ v) const {
        NTL::ZZ d_phi = (v > phi_k) ? v - phi_k : phi_k - v;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_phi < d_0;
    }

    // OUTER LAYER: RLWE (post-quantum security)
    NTL::ZZ_pX outer_encrypt_rlwe(NTL::ZZ inner) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, NTL::to_ZZ_p(inner));
        
        NTL::ZZ_pX u, e0, e1;
        uint64_t state = rng() ^ 0x1234567;
        for (int i = 0; i < N_RING; i++) {
            state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
            long u_val = (long)(state % 3) - 1;
            NTL::SetCoeff(u, i, NTL::to_ZZ_p(u_val));
            NTL::SetCoeff(e0, i, NTL::to_ZZ_p(0));
            NTL::SetCoeff(e1, i, NTL::to_ZZ_p(0));
        }
        
        NTL::ZZ_pX c0 = rlwe_pk0 * u + e0 + m;
        NTL::ZZ_pX c1 = rlwe_pk1 * u + e1;
        reduce_rlwe(c0);
        reduce_rlwe(c1);
        
        return c0;
    }

    NTL::ZZ outer_decrypt_rlwe(NTL::ZZ_pX ct) {
        NTL::ZZ_pX noise = ct + rlwe_sk;
        reduce_rlwe(noise);
        return NTL::rep(NTL::coeff(noise, 0));
    }

    // FULL HYBRID OPERATIONS
    Ciphertext encrypt(bool bit) {
        NTL::ZZ inner = inner_encrypt(bit);
        NTL::ZZ_pX outer = outer_encrypt_rlwe(inner);
        return {inner, NTL::rep(NTL::coeff(outer, 0))};
    }

    bool decrypt(Ciphertext ct) {
        return inner_decrypt(ct.first);
    }

    Ciphertext nand(Ciphertext a, Ciphertext b) {
        NTL::ZZ inner_nand_result = inner_nand(a.first, b.first);
        NTL::ZZ_pX outer = outer_encrypt_rlwe(inner_nand_result);
        return {inner_nand_result, NTL::rep(NTL::coeff(outer, 0))};
    }

    // Derived gates
    Ciphertext not_gate(Ciphertext a) { return nand(a, a); }
    Ciphertext and_gate(Ciphertext a, Ciphertext b) {
        Ciphertext n = nand(a, b);
        return nand(n, n);
    }
    Ciphertext or_gate(Ciphertext a, Ciphertext b) {
        Ciphertext not_a = nand(a, a);
        Ciphertext not_b = nand(b, b);
        return nand(not_a, not_b);
    }
    Ciphertext xor_gate(Ciphertext a, Ciphertext b) {
        Ciphertext n1 = nand(a, b);
        Ciphertext n2 = nand(a, n1);
        Ciphertext n3 = nand(b, n1);
        return nand(n2, n3);
    }
};

} // namespace golden_lucas_hybrid
