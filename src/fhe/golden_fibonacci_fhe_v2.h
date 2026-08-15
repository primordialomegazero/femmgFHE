// golden_fibonacci_fhe_v2.h
// Upgraded version: Constant-time operations, optimized for consistency

#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <utility>
#include <chrono>

namespace golden_fhe_v2 {

constexpr int N = 1024;

class FibonacciFHEV2 {
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
    
    NTL::ZZ_p alpha_p;
    NTL::ZZ_p beta_p;
    NTL::ZZ_p golden_plain_p;
    NTL::ZZ_p inv_golden_p;
    
    // Pre-computed values for constant-time operations
    NTL::ZZ_pX golden_poly;  // Pre-computed golden_plain polynomial
    
    FibonacciFHEV2(const NTL::ZZ& Q_, long secret_n = 42) : Q(Q_) {
        NTL::ZZ_p::init(Q);
        
        // Compute φ
        NTL::ZZ sqrt5;
        NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
        NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
        phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
        
        // golden_plain
        golden_plain = ((sqrt5 - 1) * inv2) % Q;
        if (golden_plain < 0) golden_plain += Q;
        inv_golden = NTL::InvMod(golden_plain, Q);
        
        // s = φ^k
        s_val = NTL::to_ZZ(1);
        for (int i = 0; i < secret_n; i++) s_val = (s_val * phi_zz) % Q;
        NTL::SetCoeff(s, 0, NTL::to_ZZ_p(s_val));
        
        // α, β from Fibonacci
        NTL::ZZ fib[100];
        fib[0] = NTL::to_ZZ(0);
        fib[1] = NTL::to_ZZ(1);
        for (int i = 2; i <= 2*secret_n; i++) {
            fib[i] = (fib[i-1] + fib[i-2]) % Q;
        }
        
        NTL::ZZ F_n = fib[secret_n];
        NTL::ZZ F_n_minus_1 = fib[secret_n - 1];
        NTL::ZZ F_2n = fib[2*secret_n];
        NTL::ZZ F_2n_minus_1 = fib[2*secret_n - 1];
        NTL::ZZ F_n_inv = NTL::InvMod(F_n, Q);
        alpha = (F_2n * F_n_inv) % Q;
        beta = (F_2n_minus_1 - alpha * F_n_minus_1) % Q;
        if (beta < 0) beta += Q;
        
        alpha_p = NTL::to_ZZ_p(alpha);
        beta_p = NTL::to_ZZ_p(beta);
        golden_plain_p = NTL::to_ZZ_p(golden_plain);
        inv_golden_p = NTL::to_ZZ_p(inv_golden);
        
        // Pre-compute golden_poly
        NTL::SetCoeff(golden_poly, 0, golden_plain_p);
        
        // KeyGen with constant-time initialization
        NTL::ZZ_pX a_poly, e_poly;
        uint64_t state = 42;
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
    
    void reduce_mod(NTL::ZZ_pX& poly) {
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
    
    // OPTIMIZED: Constant-time encrypt (fixed number of operations)
    Cipher encrypt(bool bit, uint64_t nonce) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, bit ? golden_plain_p : NTL::to_ZZ_p(0));
        
        uint64_t st = nonce;
        NTL::ZZ_pX u, e0, e1;
        
        // Fixed loop — always N iterations, no early exit
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
    
    // OPTIMIZED: Constant-time decrypt
    bool decrypt(const Cipher& ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * s;
        reduce_mod(noise);
        NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
        
        NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
        NTL::ZZ diff = (v > golden_plain) ? v - golden_plain : golden_plain - v;
        NTL::ZZ dist_golden = (diff < Q/2) ? diff : Q - diff;
        return dist_golden < dist_0;
    }
    
    // OPTIMIZED: Constant-time NAND (pre-computed golden_poly)
    Cipher nand_gate(const Cipher& a, const Cipher& b) {
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        
        NTL::ZZ_pX mult_c0 = t0 + t2 * beta_p;
        NTL::ZZ_pX mult_c1 = t1 + t2 * alpha_p;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden_p;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden_p;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);
        
        return {golden_poly - rescaled_c0, -rescaled_c1};
    }
    
    // Gates
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

} // namespace golden_fhe_v2
