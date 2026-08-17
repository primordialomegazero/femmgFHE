// SCALAR SECRET KEY RLWE
// s = φ^42 (scalar) — mas simple ang relinearization

#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "SCALAR RLWE TEST\n";
    std::cout << "================\n\n";
    
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
    
    // SCALAR SECRET KEY: s = φ^42
    NTL::ZZ s_scalar = phi_k;
    
    // α, β para sa relinearization: s² = α·s + β
    NTL::ZZ alpha = L_k;  // L(42) = φ^42 + ψ^42
    NTL::ZZ beta = Q - 1; // -1
    
    constexpr int N = 1024;
    std::mt19937_64 rng(42);
    
    // RLWE with SCALAR secret: pk = (-a·s + e, a)
    auto rlwe_encrypt = [&](NTL::ZZ msg) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, NTL::to_ZZ_p(msg));
        
        NTL::ZZ_pX a, e;
        for (int i = 0; i < N; i++) {
            NTL::SetCoeff(a, i, NTL::to_ZZ_p(rng() % NTL::conv<long>(Q)));
            NTL::SetCoeff(e, i, NTL::to_ZZ_p((rng() % 10000 == 0) ? 1 : 0));
        }
        
        // c0 = -a·s + e + m (s ay scalar)
        NTL::ZZ_pX c0 = -(a * NTL::to_ZZ_p(s_scalar)) + e + m;
        NTL::ZZ_pX c1 = a;
        return std::make_pair(c0, c1);
    };
    
    auto rlwe_decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * NTL::to_ZZ_p(s_scalar);
        return NTL::rep(NTL::coeff(noise, 0));
    };
    
    // SCALAR RELINEARIZATION: s² = α·s + β
    auto rlwe_mult = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a, 
                          std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        NTL::ZZ_pX d0 = a.first * b.first;
        NTL::ZZ_pX d1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX d2 = a.second * b.second;
        
        // Relinearization: (d0, d1, d2) → (c0, c1)
        // c0 = d0 + d2·β
        // c1 = d1 + d2·α
        NTL::ZZ_pX c0 = d0 + d2 * NTL::to_ZZ_p(beta);
        NTL::ZZ_pX c1 = d1 + d2 * NTL::to_ZZ_p(alpha);
        
        return std::make_pair(c0, c1);
    };
    
    // HOMOMORPHIC NAND
    auto rlwe_nand = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a, 
                          std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        // Multiply
        auto ab = rlwe_mult(a, b);
        
        // Scale by inv_L(k)
        NTL::ZZ_pX scaled_c0 = ab.first * NTL::to_ZZ_p(inv_L_k);
        NTL::ZZ_pX scaled_c1 = ab.second * NTL::to_ZZ_p(inv_L_k);
        
        // L(k) - scaled
        NTL::ZZ_pX L_poly;
        NTL::SetCoeff(L_poly, 0, NTL::to_ZZ_p(L_k));
        NTL::ZZ_pX result_c0 = L_poly - scaled_c0;
        NTL::ZZ_pX result_c1 = -scaled_c1;
        
        return std::make_pair(result_c0, result_c1);
    };
    
    // Test
    auto ct0 = rlwe_encrypt(NTL::to_ZZ(0));
    auto ct1 = rlwe_encrypt(L_k);
    
    std::cout << "1. ENCRYPT/DECRYPT:\n";
    std::cout << "   Decrypt(0) = " << rlwe_decrypt(ct0) << " (exp 0)\n";
    std::cout << "   Decrypt(L(k)) = " << rlwe_decrypt(ct1) << " (exp " << L_k << ")\n\n";
    
    // NAND
    auto nand00 = rlwe_nand(ct0, ct0);
    auto nand11 = rlwe_nand(ct1, ct1);
    
    std::cout << "2. NAND:\n";
    std::cout << "   NAND(0,0) = " << rlwe_decrypt(nand00) << " (exp " << L_k << ")\n";
    std::cout << "   NAND(1,1) = " << rlwe_decrypt(nand11) << " (exp 0)\n\n";
    
    // Deep chain
    std::cout << "3. DEEP CHAIN (50 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 50; i++) {
        NTL::ZZ dec_val = rlwe_decrypt(current);
        if (dec_val < 0) dec_val += Q;
        NTL::ZZ d_L = (dec_val > L_k) ? dec_val - L_k : L_k - dec_val;
        NTL::ZZ d_0 = (dec_val < Q/2) ? dec_val : Q - dec_val;
        bool dec = d_L < d_0;
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) {
            if (errors < 3) std::cout << "   Depth " << i << ": dec=" << dec << " exp=" << expected << " val=" << dec_val << "\n";
            errors++;
        }
        current = rlwe_nand(current, current);
    }
    std::cout << "   Errors: " << errors << "/51\n";
    
    return 0;
}
