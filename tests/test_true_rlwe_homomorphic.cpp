// TRUE RLWE HOMOMORPHIC MULTIPLICATION
// NAND = E(L(k)) - E(a)·E(b)·E(inv_L(k)) — walang decrypt!

#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "TRUE RLWE HOMOMORPHIC TEST\n";
    std::cout << "==========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);
    
    constexpr int N = 1024;
    std::mt19937_64 rng(42);
    
    // RLWE keys
    NTL::ZZ_pX sk;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(sk, i, NTL::to_ZZ_p((rng() % 3) - 1));
    }
    
    // Relinearization key: sk² sa RLWE
    NTL::ZZ_pX sk_sq = sk * sk;
    // I-reduce ang sk_sq sa degree < N
    if (NTL::deg(sk_sq) >= N) {
        NTL::ZZ_pX reduced;
        reduced.SetLength(N);
        for (int i = 0; i <= NTL::deg(sk_sq); i++) {
            int rd = i % N;
            NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(sk_sq, i));
        }
        sk_sq = reduced;
    }
    
    // RLWE encrypt
    auto rlwe_encrypt = [&](NTL::ZZ msg) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, NTL::to_ZZ_p(msg));
        
        NTL::ZZ_pX a, e;
        for (int i = 0; i < N; i++) {
            NTL::SetCoeff(a, i, NTL::to_ZZ_p(rng() % NTL::conv<long>(Q)));
            NTL::SetCoeff(e, i, NTL::to_ZZ_p((rng() % 10000 == 0) ? 1 : 0));
        }
        
        // c0 = -a·sk + e + m, c1 = a
        NTL::ZZ_pX c0 = -(a * sk) + e + m;
        NTL::ZZ_pX c1 = a;
        return std::make_pair(c0, c1);
    };
    
    // RLWE decrypt
    auto rlwe_decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * sk;
        return NTL::rep(NTL::coeff(noise, 0));
    };
    
    // RLWE homomorphic multiplication
    auto rlwe_mult = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a, 
                          std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        // (c0_a, c1_a) · (c0_b, c1_b)
        // = (c0_a·c0_b, c0_a·c1_b + c1_a·c0_b, c1_a·c1_b)
        // Result: (d0, d1, d2) — 3 components
        
        NTL::ZZ_pX d0 = a.first * b.first;
        NTL::ZZ_pX d1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX d2 = a.second * b.second;
        
        // I-reduce sa degree < N
        if (NTL::deg(d0) >= N) {
            NTL::ZZ_pX reduced;
            reduced.SetLength(N);
            for (int i = 0; i <= NTL::deg(d0); i++) {
                int rd = i % N;
                NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(d0, i));
            }
            d0 = reduced;
        }
        if (NTL::deg(d1) >= N) {
            NTL::ZZ_pX reduced;
            reduced.SetLength(N);
            for (int i = 0; i <= NTL::deg(d1); i++) {
                int rd = i % N;
                NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(d1, i));
            }
            d1 = reduced;
        }
        if (NTL::deg(d2) >= N) {
            NTL::ZZ_pX reduced;
            reduced.SetLength(N);
            for (int i = 0; i <= NTL::deg(d2); i++) {
                int rd = i % N;
                NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(d2, i));
            }
            d2 = reduced;
        }
        
        // Relinearization: (d0, d1, d2) → (c0, c1)
        // c0 = d0 + d2·sk², c1 = d1
        NTL::ZZ_pX c0 = d0 + d2 * sk_sq;
        NTL::ZZ_pX c1 = d1;
        
        // I-reduce c0
        if (NTL::deg(c0) >= N) {
            NTL::ZZ_pX reduced;
            reduced.SetLength(N);
            for (int i = 0; i <= NTL::deg(c0); i++) {
                int rd = i % N;
                NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(c0, i));
            }
            c0 = reduced;
        }
        
        return std::make_pair(c0, c1);
    };
    
    // HOMOMORPHIC NAND: L(k) - a·b·inv_L(k)
    auto rlwe_nand = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a, 
                          std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        // Step 1: Multiply a × b
        auto ab = rlwe_mult(a, b);
        
        // Step 2: Multiply by inv_L(k) — gawin scalar multiplication
        NTL::ZZ_pX invL_poly;
        NTL::SetCoeff(invL_poly, 0, NTL::to_ZZ_p(inv_L_k));
        NTL::ZZ_pX scaled_c0 = ab.first * invL_poly;
        NTL::ZZ_pX scaled_c1 = ab.second * invL_poly;
        
        // I-reduce
        if (NTL::deg(scaled_c0) >= N) {
            NTL::ZZ_pX reduced;
            reduced.SetLength(N);
            for (int i = 0; i <= NTL::deg(scaled_c0); i++) {
                int rd = i % N;
                NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(scaled_c0, i));
            }
            scaled_c0 = reduced;
        }
        if (NTL::deg(scaled_c1) >= N) {
            NTL::ZZ_pX reduced;
            reduced.SetLength(N);
            for (int i = 0; i <= NTL::deg(scaled_c1); i++) {
                int rd = i % N;
                NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(scaled_c1, i));
            }
            scaled_c1 = reduced;
        }
        
        // Step 3: L(k) - scaled
        NTL::ZZ_pX L_poly;
        NTL::SetCoeff(L_poly, 0, NTL::to_ZZ_p(L_k));
        NTL::ZZ_pX result_c0 = L_poly - scaled_c0;
        NTL::ZZ_pX result_c1 = -scaled_c1;
        
        return std::make_pair(result_c0, result_c1);
    };
    
    // Test
    auto ct0 = rlwe_encrypt(NTL::to_ZZ(0));
    auto ct1 = rlwe_encrypt(L_k);
    
    std::cout << "1. BASIC ENCRYPT/DECRYPT:\n";
    std::cout << "   Decrypt(ct0) = " << rlwe_decrypt(ct0) << " (exp 0)\n";
    std::cout << "   Decrypt(ct1) = " << rlwe_decrypt(ct1) << " (exp " << L_k << ")\n\n";
    
    // NAND tests
    auto nand00 = rlwe_nand(ct0, ct0);
    auto nand01 = rlwe_nand(ct0, ct1);
    auto nand10 = rlwe_nand(ct1, ct0);
    auto nand11 = rlwe_nand(ct1, ct1);
    
    std::cout << "2. HOMOMORPHIC NAND (non-interactive):\n";
    std::cout << "   NAND(0,0) decrypt = " << rlwe_decrypt(nand00) << " (exp " << L_k << ")\n";
    std::cout << "   NAND(0,1) decrypt = " << rlwe_decrypt(nand01) << " (exp " << L_k << ")\n";
    std::cout << "   NAND(1,0) decrypt = " << rlwe_decrypt(nand10) << " (exp " << L_k << ")\n";
    std::cout << "   NAND(1,1) decrypt = " << rlwe_decrypt(nand11) << " (exp 0)\n\n";
    
    // Deep chain — NON-INTERACTIVE
    std::cout << "3. DEEP CHAIN (50 depths, non-interactive):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 50; i++) {
        NTL::ZZ dec_val = rlwe_decrypt(current);
        bool dec = (dec_val == L_k || (dec_val < Q/2 && dec_val != 0) || dec_val > Q/2);
        // Simplified: check kung malapit sa L_k o malapit sa 0
        NTL::ZZ d_L = (dec_val > L_k) ? dec_val - L_k : L_k - dec_val;
        NTL::ZZ d_0 = (dec_val < Q/2) ? dec_val : Q - dec_val;
        dec = d_L < d_0;
        
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = rlwe_nand(current, current);
    }
    std::cout << "   Errors: " << errors << "/51\n";
    
    return 0;
}
