// SIMPLE NAND TEST — walang rlwe_mult, direct lang
#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <random>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) { phi_k = (phi_k * phi) % Q; psi_k = (psi_k * psi) % Q; }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);
    
    constexpr int N = 512;
    std::mt19937_64 rng(42);
    NTL::ZZ s_scalar = phi_k;
    
    // SIMPLE: Encrypt = (L_k + noise, noise)
    auto simple_encrypt = [&](bool bit) {
        NTL::ZZ_pX c0, c1;
        NTL::SetCoeff(c0, 0, NTL::to_ZZ_p(bit ? L_k : NTL::to_ZZ(0)));
        NTL::SetCoeff(c1, 0, NTL::to_ZZ_p(0));
        // Add kaunting noise sa ibang coefficients
        for (int i = 1; i < 5; i++) {
            NTL::SetCoeff(c0, i, NTL::to_ZZ_p(rng() % 100));
            NTL::SetCoeff(c1, i, NTL::to_ZZ_p(rng() % 100));
        }
        return std::make_pair(c0, c1);
    };
    
    // SIMPLE: NAND = (L_k - (a.c0·b.c0)·inv_L_k, 0)
    auto reduce_poly = [&](NTL::ZZ_pX& p) {
        if (NTL::deg(p) >= N) {
            NTL::ZZ_pX reduced;
            reduced.SetLength(N);
            for (int i = 0; i <= NTL::deg(p); i++) {
                int rd = i % (2 * N);
                if (rd >= N) {
                    rd -= N;
                    NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) - NTL::coeff(p, i));
                } else {
                    NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(p, i));
                }
            }
            p = reduced;
        }
    };
    
    auto simple_nand = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a,
                            std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        NTL::ZZ_pX prod = a.first * b.first;
        reduce_poly(prod);
        NTL::ZZ_pX scaled = prod * NTL::to_ZZ_p(inv_L_k);
        reduce_poly(scaled);
        NTL::ZZ_pX result;
        NTL::SetCoeff(result, 0, NTL::to_ZZ_p(L_k));
        result = result - scaled;
        reduce_poly(result);
        return std::make_pair(result, NTL::ZZ_pX());
    };
    
    // Test
    auto ct0 = simple_encrypt(false);
    auto ct1 = simple_encrypt(true);
    
    auto dec = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ val = NTL::rep(NTL::coeff(ct.first, 0)) % Q;
        if (val < 0) val += Q;
        NTL::ZZ d_L = (val > L_k) ? val - L_k : L_k - val;
        NTL::ZZ d_0 = (val < Q/2) ? val : Q - val;
        return d_L < d_0;
    };
    
    std::cout << "Decrypt(0) = " << dec(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(1) = " << dec(ct1) << " (exp 1)\n";
    
    // Deep chain
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 50; i++) {
        bool d = dec(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (d != expected) errors++;
        current = simple_nand(current, current);
    }
    std::cout << "Deep chain (50): " << errors << "/51 errors\n";
    
    return 0;
}
