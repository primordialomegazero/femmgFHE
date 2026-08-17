// RLWE HOMOMORPHIC NAND — 1000 DEPTHS TEST
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "RLWE HOMOMORPHIC NAND — 1000 DEPTHS\n";
    std::cout << "===================================\n\n";

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

    constexpr int N = 1024;
    std::mt19937_64 rng(42);

    NTL::ZZ_pX sk;
    NTL::SetCoeff(sk, 0, NTL::to_ZZ_p(phi_k));

    NTL::ZZ_pX a_poly;
    for (int i = 0; i < N; i++) {
        long a_val = (rng() % 3) - 1;
        NTL::SetCoeff(a_poly, i, NTL::to_ZZ_p(a_val));
    }

    NTL::ZZ_pX pk0 = -(a_poly * sk);
    NTL::ZZ_pX pk1 = a_poly;

    auto reduce_poly = [&](NTL::ZZ_pX& poly) {
        if (NTL::deg(poly) < N) return;
        NTL::ZZ_pX reduced;
        reduced.SetLength(N);
        for (int i = 0; i <= NTL::deg(poly); i++) {
            int rd = i % (2*N);
            if (rd >= N) { rd -= N; }
            NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(poly, i));
        }
        poly = reduced;
    };

    auto encrypt = [&](NTL::ZZ msg) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, NTL::to_ZZ_p(msg));
        NTL::ZZ_pX u;
        for (int i = 0; i < N; i++) {
            NTL::SetCoeff(u, i, NTL::to_ZZ_p((rng() % 3) - 1));
        }
        NTL::ZZ_pX c0 = pk0 * u + m;
        NTL::ZZ_pX c1 = pk1 * u;
        reduce_poly(c0);
        reduce_poly(c1);
        return std::make_pair(c0, c1);
    };

    auto decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * sk;
        reduce_poly(noise);
        return NTL::rep(NTL::coeff(noise, 0));
    };

    auto hom_mult = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a,
                        std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        
        // φ-natural relinearization
        NTL::ZZ_pX c0 = t0 - t2;
        NTL::ZZ_pX c1 = t1 + t2 * NTL::to_ZZ_p(L_k);
        reduce_poly(c0);
        reduce_poly(c1);
        return std::make_pair(c0, c1);
    };

    // Pre-compute E_Lk at E_inv_Lk
    auto E_Lk = encrypt(L_k);
    auto E_inv_Lk = encrypt(inv_L_k);

    auto hom_nand = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a,
                        std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        auto prod = hom_mult(a, b);
        auto scaled = hom_mult(prod, E_inv_Lk);
        // NAND = E_Lk - scaled
        NTL::ZZ_pX c0 = E_Lk.first - scaled.first;
        NTL::ZZ_pX c1 = E_Lk.second - scaled.second;
        reduce_poly(c0);
        reduce_poly(c1);
        return std::make_pair(c0, c1);
    };

    // SIMPLE TEST
    std::cout << "1. INITIAL ENCRYPTION:\n";
    auto ct_a = encrypt(NTL::to_ZZ(1));
    auto ct_b = encrypt(NTL::to_ZZ(1));

    std::cout << "   Decrypt(ct_a) = " << decrypt(ct_a) << " (exp 1)\n";
    std::cout << "   Decrypt(ct_b) = " << decrypt(ct_b) << " (exp 1)\n\n";

    std::cout << "2. NAND CHAIN (1000 DEPTHS):\n";
    auto ct_result = ct_a;
    int errors = 0;

    for (int i = 1; i <= 1000; i++) {
        ct_result = hom_nand(ct_result, ct_result); // NOT-like
        NTL::ZZ result = decrypt(ct_result);
        
        // Expected: NAND(1,1)=0, NAND(0,0)=1, alternating
        NTL::ZZ expected;
        if (i == 1) expected = NTL::to_ZZ(0);
        else if (i % 2 == 0) expected = NTL::to_ZZ(1);
        else expected = NTL::to_ZZ(0);

        if (result != expected) {
            errors++;
            if (errors <= 5) {
                std::cout << "   Depth " << i << ": got " << result 
                          << ", exp " << expected << " ✗\n";
            }
        }
        
        if (i % 100 == 0) {
            std::cout << "   Progress: " << i << "/1000, errors=" << errors << "\n";
        }
    }

    std::cout << "\n3. RESULT:\n";
    std::cout << "   Errors: " << errors << "/1000\n";

    return 0;
}
