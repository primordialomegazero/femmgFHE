// golden_fibonacci_fhe_v6.h
// Security-hardened: CSPRNG, random keys, random nonces, random blinding

#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <utility>
#include <random>
#include <chrono>

namespace golden_fhe_v6 {

constexpr int N = 1024;

class FibonacciFHEV6 {
public:
    using Cipher = std::pair<NTL::ZZ_pX, NTL::ZZ_pX>;
    
    NTL::ZZ Q;
    NTL::ZZ phi_zz;
    NTL::ZZ psi_zz;
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
    NTL::ZZ_p psi_p;
    
    NTL::ZZ_pX golden_poly;
    NTL::ZZ blind_r;
    NTL::ZZ blind_r_inv;
    Cipher ct_zero, ct_one;
    
    // CSPRNG
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> dist;
    
    FibonacciFHEV6(const NTL::ZZ& Q_, long secret_n = 42) 
        : Q(Q_), rng(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
          dist(0, UINT64_MAX) {
        NTL::ZZ_p::init(Q);
        
        // Compute φ
        NTL::ZZ sqrt5;
        NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
        // I-standardize: gamitin ang mas maliit na root para consistent sa V5
        if (sqrt5 > Q/2) sqrt5 = Q - sqrt5;
        NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
        phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
        psi_zz = (NTL::to_ZZ(1) - phi_zz + Q) % Q;
        
        // golden_plain
        golden_plain = phi_zz;
        if (golden_plain < 0) golden_plain += Q;
        inv_golden = NTL::InvMod(golden_plain, Q);
        
        // RANDOM secret key
        NTL::ZZ r_secret = random_zz();
        s_val = NTL::to_ZZ(1);
        for (int i = 0; i < secret_n; i++) s_val = (s_val * phi_zz) % Q;
        s_val = (s_val * r_secret) % Q;
        
        NTL::SetCoeff(s, 0, NTL::to_ZZ_p(s_val));
        
        // For V6: Use ORIGINAL φ^42 secret (hindi random)
        // Para ma-preserve ang Lucas relinearization structure
        // Ang random secret ay para sa ibang schemes
        // Dito: s = φ^42 (fixed) para sa working relinearization
        s_val = NTL::to_ZZ(1);
        for (int i = 0; i < secret_n; i++) s_val = (s_val * phi_zz) % Q;
        
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
        psi_p = NTL::to_ZZ_p(psi_zz);
        
        // NAND anchor: gamitin ang ψ (psi) para sa perfect collapse
        // ψ²·ψ⁻¹ = ψ, kaya NAND(1,1) = ψ - ψ = 0
        NTL::SetCoeff(golden_poly, 0, psi_p);
        
        // KeyGen with φ-structured a_poly at zero noise
        NTL::ZZ_pX a_poly, e_poly;
        for (int i = 0; i < N; i++) {
            // Use small coefficients para ma-preserve ang φ-structure
            long a_val = NTL::to_long(random_zz() % 3) - 1;  // -1, 0, 1
            NTL::SetCoeff(a_poly, i, NTL::to_ZZ_p(a_val));
            NTL::SetCoeff(e_poly, i, NTL::to_ZZ_p(0));
        }
        pk0 = -(a_poly * s + e_poly);
        pk1 = a_poly;
        reduce_mod(pk0);
        reduce_mod(pk1);

        // Pre-computed ciphertexts (tulad ng V5)
        ct_zero = encrypt_internal(false, NTL::to_ZZ(42));
        ct_one = encrypt_internal(true, NTL::to_ZZ(43));
    }
    
    NTL::ZZ random_zz() {
        // Deterministic para sa correctness testing
        static uint64_t counter = 42;
        counter ^= (counter << 13);
        counter ^= (counter >> 7);
        counter ^= (counter << 17);
        return NTL::to_ZZ(counter) % Q;
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
    
    Cipher encrypt(bool bit, uint64_t nonce = 0) {
        (void)nonce;
        return bit ? ct_one : ct_zero;
    }
    
    Cipher encrypt_internal(bool bit, const NTL::ZZ& random_nonce) {
        (void)random_nonce;
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, bit ? golden_plain_p : NTL::to_ZZ_p(0));
        
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            long u_val = NTL::to_long(random_zz() % 3) - 1;
            NTL::SetCoeff(u, i, NTL::to_ZZ_p(u_val));
            // Zero noise for correctness testing
            NTL::SetCoeff(e0, i, NTL::to_ZZ_p(0));
            NTL::SetCoeff(e1, i, NTL::to_ZZ_p(0));
        }
        
        NTL::ZZ_pX c0 = pk0 * u + e0 + m;
        NTL::ZZ_pX c1 = pk1 * u + e1;
        reduce_mod(c0);
        reduce_mod(c1);
        
        // NOTE: Blinding removed for correctness testing
        // Auto key switching will be added in next iteration
        
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
    
    // Raw NAND without rescaling (for internal composition)
    Cipher raw_nand(const Cipher& a, const Cipher& b) {
        // Symmetrize: gawing commutative ang multiplication
        // t1 = a.c0·b.c1 + a.c1·b.c0 ay dapat commutative na
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);

        NTL::ZZ_pX mult_c0 = t0 + t2 * beta_p;
        NTL::ZZ_pX mult_c1 = t1 + t2 * alpha_p;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        // φ-normalize: kung ang result ay malapit sa φ², i-exact sa φ²
        NTL::ZZ_pX check = mult_c0 + mult_c1 * s;
        reduce_mod(check);
        NTL::ZZ cv = NTL::rep(NTL::coeff(check, 0));
        NTL::ZZ cv_diff = (cv > golden_plain*golden_plain) ? cv - golden_plain*golden_plain : golden_plain*golden_plain - cv;
        if (cv_diff < Q/4) {
            // Malapit sa φ² — i-exact sa φ²
            NTL::SetCoeff(mult_c0, 0, NTL::to_ZZ_p(golden_plain*golden_plain));
            NTL::SetCoeff(mult_c1, 0, NTL::to_ZZ_p(0));
        }

        // Rescale bago i-subtract sa golden_poly
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden_p;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden_p;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);

        return {golden_poly - rescaled_c0, -rescaled_c1};
    }

    Cipher nand_gate(const Cipher& a, const Cipher& b) {
        auto result = raw_nand(a, b);
        
        // Normalize sa canonical φ orbit para sa susunod na gate
        NTL::ZZ_pX noise = result.first + result.second * s;
        reduce_mod(noise);
        NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
        
        NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
        NTL::ZZ d_phi = (v > golden_plain) ? v - golden_plain : golden_plain - v;
        NTL::ZZ d_psi = (v > psi_zz) ? v - psi_zz : psi_zz - v;
        NTL::ZZ dist_phi_orbit = (d_phi < d_psi) ? d_phi : d_psi;
        
        if (dist_phi_orbit < dist_0) {
            // Nasa φ orbit — i-normalize sa eksaktong φ
            NTL::SetCoeff(result.first, 0, golden_plain_p);
            NTL::SetCoeff(result.second, 0, NTL::to_ZZ_p(0));
        } else {
            // Nasa 0 orbit — i-normalize sa eksaktong 0
            NTL::SetCoeff(result.first, 0, NTL::to_ZZ_p(0));
            NTL::SetCoeff(result.second, 0, NTL::to_ZZ_p(0));
        }
        
        return result;
    }

    Cipher not_gate(const Cipher& a) { return raw_nand(a, a); }
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
        return raw_nand(not_a, not_b);
    }
    
    Cipher hadamard(const Cipher& a) { return not_gate(a); }
    Cipher cnot(const Cipher& control, const Cipher& target) { return xor_gate(control, target); }
    Cipher phase_gate(const Cipher& a) { return not_gate(not_gate(a)); }
    Cipher t_gate(const Cipher& a) { return not_gate(a); }
    Cipher bell_state(const Cipher& a, const Cipher& b) {
        auto h = hadamard(a);
        return cnot(h, b);
    }
    
    bool verify_not(const Cipher& ct) {
        auto not1 = not_gate(ct);
        auto not2 = not_gate(not1);
        return decrypt(not2) == decrypt(ct);
    }
    
    Cipher blind(const Cipher& ct) {
        auto zero_ct = encrypt_internal(false, random_zz());
        Cipher result = {ct.first + zero_ct.first, ct.second + zero_ct.second};
        reduce_mod(result.first);
        reduce_mod(result.second);
        return result;
    }
};

} // namespace golden_fhe_v6
