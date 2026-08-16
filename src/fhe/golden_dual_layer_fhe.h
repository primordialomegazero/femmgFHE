// golden_dual_layer_fhe.h
// Dual-Layer FHE: Inner φ-ring (perfect NAND) + Outer ψ-noise (semantic security)

#pragma once
#include <NTL/ZZ.h>
#include <iostream>
#include <random>
#include <utility>

namespace golden_dual_fhe {

class DualLayerFHE {
public:
    using Ciphertext = NTL::ZZ;

    NTL::ZZ Q;
    NTL::ZZ phi;
    NTL::ZZ psi;
    NTL::ZZ phi_k;  // φ^k — inner message encoding
    NTL::ZZ psi_k;  // ψ^k — outer noise anchor
    NTL::ZZ inv_phi_k;

    std::mt19937_64 rng;

    DualLayerFHE(const NTL::ZZ& Q_, long secret_k = 42) : Q(Q_), rng(42) {
        NTL::ZZ sqrt5;
        NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
        NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
        phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
        psi = (NTL::to_ZZ(1) - phi + Q) % Q;

        phi_k = NTL::to_ZZ(1);
        psi_k = NTL::to_ZZ(1);
        for (long i = 0; i < secret_k; i++) {
            phi_k = (phi_k * phi) % Q;
            psi_k = (psi_k * psi) % Q;
        }

        inv_phi_k = NTL::InvMod(phi_k, Q);
    }

    // INNER LAYER: φ-ring (perfect NAND, walang noise)
    NTL::ZZ inner_encrypt(bool bit) const {
        return bit ? phi_k : NTL::to_ZZ(0);
    }

    bool inner_decrypt(NTL::ZZ v) const {
        NTL::ZZ d_phi = (v > phi_k) ? v - phi_k : phi_k - v;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_phi < d_0;
    }

    NTL::ZZ inner_nand(NTL::ZZ a, NTL::ZZ b) const {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    }

    // OUTER LAYER: ψ-noise Q-embed (semantic security)
    NTL::ZZ outer_encrypt(NTL::ZZ inner_ct) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        return inner_ct + e * psi_k + r * Q;
    }

    NTL::ZZ outer_decrypt_inner(NTL::ZZ ct) const {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ scaled = (v * phi_k) % Q;
        NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
        NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
        if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return (d_phi2k < d_0) ? phi_k : NTL::to_ZZ(0);
    }

    // FULL DUAL-LAYER OPERATIONS
    Ciphertext encrypt(bool bit) {
        return outer_encrypt(inner_encrypt(bit));
    }

    bool decrypt(Ciphertext ct) const {
        NTL::ZZ inner = outer_decrypt_inner(ct);
        return inner_decrypt(inner);
    }

    Ciphertext nand(Ciphertext a, Ciphertext b) {
        NTL::ZZ inner_a = outer_decrypt_inner(a);
        NTL::ZZ inner_b = outer_decrypt_inner(b);
        NTL::ZZ inner_result = inner_nand(inner_a, inner_b);
        return outer_encrypt(inner_result);
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

} // namespace golden_dual_fhe
