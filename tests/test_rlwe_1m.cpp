// RLWE L(k) FHE — 1M DEPTH TEST
// Every 100K print

#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "1M DEPTH TEST\n";
    std::cout << "=============\n\n";
    
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
    
    constexpr int N = 512;
    std::mt19937_64 rng(42);
    
    NTL::ZZ s_scalar = phi_k;
    NTL::ZZ alpha = L_k;
    NTL::ZZ beta = Q - 1;
    
    NTL::ZZ_pX a_pk, e_pk;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(a_pk, i, NTL::to_ZZ_p(NTL::RandomBnd(Q)));
        NTL::SetCoeff(e_pk, i, NTL::to_ZZ_p((rng() % 100000 == 0) ? 1 : 0));
    }
    NTL::ZZ_pX pk0 = -(a_pk * NTL::to_ZZ_p(s_scalar)) + e_pk;
    NTL::ZZ_pX pk1 = a_pk;
    
    auto reduce_poly = [&](NTL::ZZ_pX& p) {
        if (NTL::deg(p) >= N) {
            NTL::ZZ_pX reduced;
            reduced.SetLength(N);
            for (int i = 0; i <= NTL::deg(p); i++) {
                int rd = i % (2 * N);
                if (rd >= N) {
                    rd -= N;
                    // x^N = -1, kaya i-SUBTRACT
                    NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) - NTL::coeff(p, i));
                } else {
                    NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(p, i));
                }
            }
            p = reduced;
        }
    };
    
    auto rlwe_encrypt = [&](NTL::ZZ msg) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, NTL::to_ZZ_p(msg));
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            NTL::SetCoeff(u, i, NTL::to_ZZ_p((rng() % 3) - 1));
            NTL::SetCoeff(e0, i, NTL::to_ZZ_p((rng() % 100000 == 0) ? 1 : 0));
            NTL::SetCoeff(e1, i, NTL::to_ZZ_p((rng() % 100000 == 0) ? 1 : 0));
        }
        NTL::ZZ_pX c0 = pk0 * u + e0 + m;
        NTL::ZZ_pX c1 = pk1 * u + e1;
        reduce_poly(c0);
        reduce_poly(c1);
        return std::make_pair(c0, c1);
    };
    
    auto rlwe_decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * NTL::to_ZZ_p(s_scalar);
        reduce_poly(noise);
        return NTL::rep(NTL::coeff(noise, 0));
    };
    
    auto rlwe_mult = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a, 
                          std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        NTL::ZZ_pX d0 = a.first * b.first;
        NTL::ZZ_pX d1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX d2 = a.second * b.second;
        reduce_poly(d0);
        reduce_poly(d1);
        reduce_poly(d2);
        
        // I-scale ang beta at alpha bilang ZZ_p
        NTL::ZZ_p beta_p = NTL::to_ZZ_p(beta);
        NTL::ZZ_p alpha_p = NTL::to_ZZ_p(alpha);
        
        NTL::ZZ_pX c0 = d0 + d2 * beta_p;
        NTL::ZZ_pX c1 = d1 + d2 * alpha_p;
        reduce_poly(c0);
        reduce_poly(c1);
        return std::make_pair(c0, c1);
    };
    
    auto enc_L = rlwe_encrypt(L_k);
    
    auto homomorphic_nand = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a,
                                 std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        auto ab = rlwe_mult(a, b);
        NTL::ZZ_p invL_p = NTL::to_ZZ_p(inv_L_k);
        
        // I-scale ang ab.first at ab.second
        NTL::ZZ_pX scaled_c0 = ab.first * invL_p;
        NTL::ZZ_pX scaled_c1 = ab.second * invL_p;
        reduce_poly(scaled_c0);
        reduce_poly(scaled_c1);
        
        NTL::ZZ_pX result_c0 = enc_L.first - scaled_c0;
        NTL::ZZ_pX result_c1 = enc_L.second - scaled_c1;
        reduce_poly(result_c0);
        reduce_poly(result_c1);
        return std::make_pair(result_c0, result_c1);
    };
    
    auto full_encrypt = [&](bool bit) {
        return rlwe_encrypt(bit ? L_k : NTL::to_ZZ(0));
    };
    
    auto full_decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ val = rlwe_decrypt(ct) % Q;
        if (val < 0) val += Q;
        NTL::ZZ d_L = (val > L_k) ? val - L_k : L_k - val;
        NTL::ZZ d_0 = (val < Q/2) ? val : Q - val;
        return d_L < d_0;
    };
    
    // 1M depth test
    std::cout << "1M DEPTH CHAIN (every 100K):\n";
    auto current = full_encrypt(true);
    int errors = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::cout << "Starting loop..." << std::endl;
    std::cout.flush();
    for (int i = 0; i <= 1000; i++) {
        if (i < 5) { std::cout << "  i=" << i << std::endl; std::cout.flush(); }
        bool dec = full_decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = homomorphic_nand(current, current);
        
        if (i % 100 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            std::cout << "  [" << i << "/1M] errors=" << errors 
                      << " time=" << elapsed << "s (" << (i / elapsed) << " ops/sec)\n";
            std::cout.flush();
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    
    std::cout << "\nFINAL: errors=" << errors << "/1000000\n";
    std::cout << "Total time: " << total << "s\n";
    
    return 0;
}
