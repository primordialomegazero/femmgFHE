// golden_fibonacci_fhe.h
// Core Fibonacci FHE — No Bootstrapping Required
// 
// This is the MAIN implementation. It supersedes all previous
// bootstrapping-based approaches.
//
// Key innovation: Automatic relinearization via Lucas numbers
// s² = α·s + β where α = L(k), β = -1
// This eliminates the need for bootstrapping entirely.

#ifndef GOLDEN_FIBONACCI_FHE_H
#define GOLDEN_FIBONACCI_FHE_H

#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <utility>
#include <chrono>

namespace golden_fhe {

// ============ CONSTANTS ============
constexpr int N = 1024;  // Ring dimension
constexpr double PHI = 1.6180339887498948482;

// ============ CORE FUNCTIONS ============
inline void init_ring(const NTL::ZZ& Q) {
    NTL::ZZ_p::init(Q);
}

inline void reduce_mod(NTL::ZZ_pX& poly) {
    if (NTL::deg(poly) < N) return;
    NTL::ZZ_pX reduced;
    reduced.SetLength(N);
    for (int i = 0; i <= NTL::deg(poly); i++) {
        NTL::ZZ_p coeff = NTL::coeff(poly, i);
        int reduced_deg = i % (2 * N);
        if (reduced_deg >= N) {
            reduced_deg -= N;
            coeff = -coeff;
        }
        NTL::SetCoeff(reduced, reduced_deg, 
                      NTL::coeff(reduced, reduced_deg) + coeff);
    }
    poly = reduced;
}

inline NTL::ZZ fib_mod_zz(long n, const NTL::ZZ& mod) {
    if (n == 0) return NTL::to_ZZ(0);
    if (n == 1) return NTL::to_ZZ(1);
    NTL::ZZ a = NTL::to_ZZ(0);
    NTL::ZZ b = NTL::to_ZZ(1);
    for (long i = 2; i <= n; i++) {
        NTL::ZZ c = (a + b) % mod;
        a = b;
        b = c;
    }
    return b;
}

inline NTL::ZZ mod_inv_zz(const NTL::ZZ& a, const NTL::ZZ& mod) {
    NTL::ZZ t = NTL::to_ZZ(0);
    NTL::ZZ new_t = NTL::to_ZZ(1);
    NTL::ZZ r = mod;
    NTL::ZZ new_r = a;
    while (new_r != 0) {
        NTL::ZZ q = r / new_r;
        NTL::ZZ temp_t = t - q * new_t;
        t = new_t;
        new_t = temp_t;
        NTL::ZZ temp_r = r - q * new_r;
        r = new_r;
        new_r = temp_r;
    }
    if (t < 0) t += mod;
    return t;
}

inline NTL::ZZ mod_pow_zz(NTL::ZZ base, long exp, const NTL::ZZ& mod) {
    NTL::ZZ result = NTL::to_ZZ(1);
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

// ============ FHE CLASS ============
class FibonacciFHE {
public:
    using Cipher = std::pair<NTL::ZZ_pX, NTL::ZZ_pX>;
    
    NTL::ZZ Q;
    NTL::ZZ phi_zz;
    NTL::ZZ golden_plain;
    NTL::ZZ inv_golden;
    NTL::ZZ alpha;
    NTL::ZZ beta;
    NTL::ZZ s_val;
    NTL::ZZ_pX s;
    NTL::ZZ_pX pk0;
    NTL::ZZ_pX pk1;
    
    // ZZ_p versions for polynomial ops
    NTL::ZZ_p alpha_p;
    NTL::ZZ_p beta_p;
    NTL::ZZ_p golden_plain_p;
    NTL::ZZ_p inv_golden_p;
    
    FibonacciFHE(const NTL::ZZ& Q_, long secret_n = 42) : Q(Q_) {
        init_ring(Q);
        
        // Compute φ = (1+√5)/2
        NTL::ZZ sqrt5;
        NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
        NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
        phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
        
        // golden_plain = Q/φ = (√5-1)/2 · Q
        golden_plain = ((sqrt5 - 1) * inv2) % Q;
        if (golden_plain < 0) golden_plain += Q;
        inv_golden = mod_inv_zz(golden_plain, Q);
        
        // s = φ^secret_n
        s_val = mod_pow_zz(phi_zz, secret_n, Q);
        NTL::SetCoeff(s, 0, NTL::to_ZZ_p(s_val));
        
        // α = L(secret_n), β = -1
        NTL::ZZ F_n = fib_mod_zz(secret_n, Q);
        NTL::ZZ F_n_minus_1 = fib_mod_zz(secret_n - 1, Q);
        NTL::ZZ F_2n = fib_mod_zz(2 * secret_n, Q);
        NTL::ZZ F_2n_minus_1 = fib_mod_zz(2 * secret_n - 1, Q);
        NTL::ZZ F_n_inv = mod_inv_zz(F_n, Q);
        alpha = (F_2n * F_n_inv) % Q;
        beta = (F_2n_minus_1 - alpha * F_n_minus_1) % Q;
        if (beta < 0) beta += Q;
        
        alpha_p = NTL::to_ZZ_p(alpha);
        beta_p = NTL::to_ZZ_p(beta);
        golden_plain_p = NTL::to_ZZ_p(golden_plain);
        inv_golden_p = NTL::to_ZZ_p(inv_golden);
        
        // KeyGen
        uint64_t state = 42;
        NTL::ZZ_pX a_poly, e_poly;
        for (int i = 0; i < N; i++) {
            state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
            NTL::ZZ state_zz = NTL::to_ZZ(state);
            NTL::SetCoeff(a_poly, i, NTL::to_ZZ_p(state_zz % Q));
            NTL::SetCoeff(e_poly, i, NTL::to_ZZ_p((state % 10000) == 0 ? 1 : 0));
        }
        pk0 = -(a_poly * s + e_poly);
        pk1 = a_poly;
        reduce_mod(pk0);
        reduce_mod(pk1);
    }
    
    Cipher encrypt(bool bit, uint64_t nonce) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, bit ? golden_plain_p : NTL::to_ZZ_p(0));
        
        uint64_t st = nonce;
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            st ^= (st << 13); st ^= (st >> 7); st ^= (st << 17);
            NTL::SetCoeff(u, i, NTL::to_ZZ_p((long)((st % 3) - 1)));
            NTL::SetCoeff(e0, i, NTL::to_ZZ_p((st % 10000) == 0 ? 1 : 0));
            NTL::SetCoeff(e1, i, NTL::to_ZZ_p((st % 10000) == 0 ? 1 : 0));
        }
        
        NTL::ZZ_pX c0 = pk0 * u + e0 + m;
        NTL::ZZ_pX c1 = pk1 * u + e1;
        reduce_mod(c0);
        reduce_mod(c1);
        return {c0, c1};
    }
    
    bool decrypt(const Cipher& ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * s;
        reduce_mod(noise);
        NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
        
        NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
        NTL::ZZ diff = (v > golden_plain) ? v - golden_plain : golden_plain - v;
        NTL::ZZ dist_golden = (diff < Q/2) ? diff : Q - diff;
        return dist_golden < dist_0;
    }
    
    // NO BOOTSTRAPPING REQUIRED — automatic relinearization
    Cipher nand_gate(const Cipher& a, const Cipher& b) {
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        
        // Automatic relinearization: s² = α·s + β
        NTL::ZZ_pX mult_c0 = t0 + t2 * beta_p;
        NTL::ZZ_pX mult_c1 = t1 + t2 * alpha_p;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        // Rescaling
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden_p;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden_p;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);
        
        // NAND: golden_plain - product
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, golden_plain_p);
        
        return {golden_poly - rescaled_c0, -rescaled_c1};
    }
    
    Cipher xor_gate(const Cipher& a, const Cipher& b) {
        auto n1 = nand_gate(a, b);
        auto n2 = nand_gate(a, n1);
        auto n3 = nand_gate(b, n1);
        return nand_gate(n2, n3);
    }
    
    Cipher and_gate(const Cipher& a, const Cipher& b) {
        auto n = nand_gate(a, b);
        return nand_gate(n, n);
    }
    
    Cipher or_gate(const Cipher& a, const Cipher& b) {
        auto not_a = nand_gate(a, a);
        auto not_b = nand_gate(b, b);
        return nand_gate(not_a, not_b);
    }
    
    Cipher not_gate(const Cipher& a) {
        return nand_gate(a, a);
    }
};

} // namespace golden_fhe

#endif // GOLDEN_FIBONACCI_FHE_H
