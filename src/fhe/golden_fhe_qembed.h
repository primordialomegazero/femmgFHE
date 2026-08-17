// golden_fhe_qembed.h
// Clean implementation: Q-Embedding FHE with φ^k public key

#pragma once
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <iostream>
#include <random>
#include <chrono>

namespace golden_fhe_qembed {

constexpr int N = 1024;

class QEmbedFHE {
public:
    NTL::ZZ Q;
    NTL::ZZ phi;
    NTL::ZZ psi;
    NTL::ZZ s;         // Secret: s = φ^k
    NTL::ZZ inv_s;
    NTL::ZZ public_s;  // Public: s (same, pero ang SECRET ay φ at k)
    long secret_k;
    
    std::mt19937_64 rng;
    
    QEmbedFHE(const NTL::ZZ& Q_, long k = 42) : Q(Q_), secret_k(k) {
        NTL::ZZ_p::init(Q);
        
        // Seed rng
        rng.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        
        // Compute φ
        NTL::ZZ sqrt5;
        NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
        NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
        phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
        psi = (NTL::to_ZZ(1) - phi + Q) % Q;
        
        // s = φ^k
        s = NTL::to_ZZ(1);
        for (int i = 0; i < k; i++) s = (s * phi) % Q;
        inv_s = NTL::InvMod(s, Q);
        public_s = s;
    }
    
    NTL::ZZ random_r() {
        // Malaking random r (~same size sa Q) para ma-hide ang pattern
        return NTL::RandomBnd(Q);
    }
    
    // Encrypt: ct = m·s + r·Q
    NTL::ZZ encrypt(bool bit) {
        NTL::ZZ r = random_r();
        NTL::ZZ m_val = bit ? s : NTL::to_ZZ(0);
        return m_val + r * Q;
    }
    
    // Decrypt: v mod Q → check dist sa s
    bool decrypt(NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ d_s = (v > s) ? v - s : s - v;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_s < d_0;
    }
    
    // NAND: s - (a·b mod Q)·s⁻¹, then re-embed sa Q
    NTL::ZZ nand(NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        NTL::ZZ b_mod = b % Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_s) % Q;
        NTL::ZZ result = (s - scaled) % Q;
        if (result < 0) result += Q;
        NTL::ZZ r = random_r();
        return result + r * Q;
    }
    
    NTL::ZZ not_gate(NTL::ZZ a) { return nand(a, a); }
    NTL::ZZ xor_gate(NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ n1 = nand(a, b);
        NTL::ZZ n2 = nand(a, n1);
        NTL::ZZ n3 = nand(b, n1);
        return nand(n2, n3);
    }
    NTL::ZZ and_gate(NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ n = nand(a, b);
        return nand(n, n);
    }
    NTL::ZZ or_gate(NTL::ZZ a, NTL::ZZ b) {
        return nand(nand(a, a), nand(b, b));
    }
};

} // namespace golden_fhe_qembed
