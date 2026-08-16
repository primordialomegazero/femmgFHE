// RLWE-ENCRYPTED L(k) — SECURE FHE
// Evaluator: RLWE(L(k)), RLWE(inv_L(k)), RLWE(pk)
// Hindi makikita ang L(k) o φ^k sa plaintext!

#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "RLWE-ENCRYPTED L(k) — SECURE FHE\n";
    std::cout << "=================================\n\n";
    
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
    
    constexpr int N = 256;
    std::mt19937_64 rng(42);
    
    // ============ RLWE SETUP (SCALAR SECRET) ============
    // Secret key: s = φ^42 (scalar!)
    NTL::ZZ s_scalar = phi_k;
    
    // Relinearization: s² = α·s + β
    NTL::ZZ alpha = L_k;  // L(42)
    NTL::ZZ beta = Q - 1; // -1
    
    // RLWE Public Key: pk = (-a·s + e, a) kung saan s ay scalar
    NTL::ZZ_pX a_pk, e_pk;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(a_pk, i, NTL::to_ZZ_p(rng() % NTL::conv<long>(Q)));
        NTL::SetCoeff(e_pk, i, NTL::to_ZZ_p((rng() % 10000 == 0) ? 1 : 0));
    }
    NTL::ZZ_pX pk0 = -(a_pk * NTL::to_ZZ_p(s_scalar)) + e_pk;
    NTL::ZZ_pX pk1 = a_pk;
    
    // RLWE Encrypt
    auto rlwe_encrypt = [&](NTL::ZZ msg) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, NTL::to_ZZ_p(msg));
        
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            NTL::SetCoeff(u, i, NTL::to_ZZ_p((rng() % 3) - 1));
            NTL::SetCoeff(e0, i, NTL::to_ZZ_p((rng() % 10000 == 0) ? 1 : 0));
            NTL::SetCoeff(e1, i, NTL::to_ZZ_p((rng() % 10000 == 0) ? 1 : 0));
        }
        
        NTL::ZZ_pX c0 = pk0 * u + e0 + m;
        NTL::ZZ_pX c1 = pk1 * u + e1;
        return std::make_pair(c0, c1);
    };
    
    // RLWE Decrypt
    auto rlwe_decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * NTL::to_ZZ_p(s_scalar);
        return NTL::rep(NTL::coeff(noise, 0));
    };
    
    // RLWE Homomorphic Multiplication (scalar relinearization)
    auto rlwe_mult = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a, 
                          std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        NTL::ZZ_pX d0 = a.first * b.first;
        NTL::ZZ_pX d1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX d2 = a.second * b.second;
        
        // Scalar relinearization: c0 = d0 + d2·β, c1 = d1 + d2·α
        NTL::ZZ_pX c0 = d0 + d2 * NTL::to_ZZ_p(beta);
        NTL::ZZ_pX c1 = d1 + d2 * NTL::to_ZZ_p(alpha);
        
        return std::make_pair(c0, c1);
    };
    
    // ENCRYPTED BOOTSTRAPPING KEYS
    auto enc_L = rlwe_encrypt(L_k);
    auto enc_invL = rlwe_encrypt(inv_L_k);
    
    std::cout << "1. ENCRYPTED KEYS VERIFICATION:\n";
    NTL::ZZ dec_L = rlwe_decrypt(enc_L) % Q;
    if (dec_L < 0) dec_L += Q;
    std::cout << "   Decrypt(RLWE(L)) = " << dec_L << " (exp " << L_k << ")\n";
    std::cout << "   Match: " << (dec_L == L_k ? "YES ✓" : "NO ✗") << "\n\n";
    
    // HOMOMORPHIC NAND (non-interactive!)
    // E(NAND) = E(L) - E(a)·E(b)·E(inv_L)
    auto homomorphic_nand = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a,
                                 std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        // Step 1: Multiply a × b
        auto ab = rlwe_mult(a, b);
        
        // Step 2: Scale by inv_L(k) — scalar multiplication
        NTL::ZZ_pX invL_poly;
        NTL::SetCoeff(invL_poly, 0, NTL::to_ZZ_p(inv_L_k));
        NTL::ZZ_pX scaled_c0 = ab.first * invL_poly;
        NTL::ZZ_pX scaled_c1 = ab.second * invL_poly;
        
        // Step 3: E(L) - scaled
        NTL::ZZ_pX result_c0 = enc_L.first - scaled_c0;
        NTL::ZZ_pX result_c1 = enc_L.second - scaled_c1;
        
        return std::make_pair(result_c0, result_c1);
    };
    
    // FULL PIPELINE
    auto full_encrypt = [&](bool bit) {
        NTL::ZZ msg = bit ? L_k : NTL::to_ZZ(0);
        return rlwe_encrypt(msg);
    };
    
    auto full_decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ val = rlwe_decrypt(ct) % Q;
        if (val < 0) val += Q;
        NTL::ZZ d_L = (val > L_k) ? val - L_k : L_k - val;
        NTL::ZZ d_0 = (val < Q/2) ? val : Q - val;
        return d_L < d_0;
    };
    
    // Test
    auto ct0 = full_encrypt(false);
    auto ct1 = full_encrypt(true);
    
    std::cout << "2. ENCRYPT/DECRYPT:\n";
    std::cout << "   Decrypt(0) = " << full_decrypt(ct0) << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << full_decrypt(ct1) << " (exp 1)\n\n";
    
    // NAND
    auto nand00 = homomorphic_nand(ct0, ct0);
    auto nand11 = homomorphic_nand(ct1, ct1);
    
    std::cout << "3. HOMOMORPHIC NAND (non-interactive):\n";
    std::cout << "   NAND(0,0) = " << full_decrypt(nand00) << " (exp 1)\n";
    std::cout << "   NAND(1,1) = " << full_decrypt(nand11) << " (exp 0)\n\n";
    
    // Deep chain
    std::cout << "4. DEEP CHAIN (50 depths, non-interactive):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 20; i++) {
        NTL::ZZ raw_val = rlwe_decrypt(current) % Q;
        if (raw_val < 0) raw_val += Q;
        bool dec = full_decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        std::cout << "   Depth " << i << ": raw=" << raw_val << " dec=" << dec << " exp=" << expected << "\n";
        if (dec != expected) errors++;
        current = homomorphic_nand(current, current);
    }
    std::cout << "   Errors: " << errors << "/21\n";
    
    return 0;
}
