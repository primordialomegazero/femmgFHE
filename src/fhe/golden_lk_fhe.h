// golden_lk_fhe.h
// L(k)-Based Non-Interactive FHE
// Public: Q, L(k), inv_L(k)
// Secret: φ^k, ψ^k
// NAND: L(k) - a·b·inv_L(k)

#pragma once
#include <NTL/ZZ.h>
#include <iostream>
#include <random>
#include <utility>

namespace golden_lk_fhe {

class LkFHE {
public:
    using Ciphertext = NTL::ZZ;

    NTL::ZZ Q;
    NTL::ZZ phi;
    NTL::ZZ psi;
    NTL::ZZ phi_k;
    NTL::ZZ psi_k;
    NTL::ZZ L_k;       // PUBLIC
    NTL::ZZ inv_L_k;   // PUBLIC

    std::mt19937_64 rng;

    LkFHE(const NTL::ZZ& Q_, long k = 42) : Q(Q_), rng(42) {
        NTL::ZZ sqrt5;
        NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
        NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
        phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
        psi = (NTL::to_ZZ(1) - phi + Q) % Q;

        phi_k = NTL::to_ZZ(1);
        psi_k = NTL::to_ZZ(1);
        for (long i = 0; i < k; i++) {
            phi_k = (phi_k * phi) % Q;
            psi_k = (psi_k * psi) % Q;
        }
        L_k = (phi_k + psi_k) % Q;
        inv_L_k = NTL::InvMod(L_k, Q);
    }

    // Encrypt: ct = m·L(k) + r·Q
    Ciphertext encrypt(bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ m_val = bit ? L_k : NTL::to_ZZ(0);
        return m_val + r * Q;
    }

    // Decrypt: v = ct mod Q, check kung L(k) o 0
    bool decrypt(Ciphertext ct) const {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ diff_L = (v > L_k) ? v - L_k : L_k - v;
        if (diff_L > Q/2) diff_L = Q - diff_L;
        NTL::ZZ diff_0 = (v < Q/2) ? v : Q - v;
        return diff_L < diff_0;
    }

    // NAND: L(k) - (a·b)·inv_L(k)
    Ciphertext nand(Ciphertext a, Ciphertext b) const {
        NTL::ZZ a_mod = a % Q;
        if (a_mod < 0) a_mod += Q;
        NTL::ZZ b_mod = b % Q;
        if (b_mod < 0) b_mod += Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_L_k) % Q;
        NTL::ZZ result = (L_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
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

} // namespace golden_lk_fhe
