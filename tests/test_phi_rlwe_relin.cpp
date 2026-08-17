// φ-NATURAL RELINEARIZATION SA RLWE
// s = φ^k (scalar), s² = L(k)·s - 1 (natural!)

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "φ-NATURAL RELINEARIZATION SA RLWE\n";
    std::cout << "================================\n\n";

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

    std::cout << "1. ANG KEY IDEA:\n";
    std::cout << "   RLWE secret key: s = φ^k (SCALAR, hindi polynomial)\n";
    std::cout << "   s² = φ^(2k) = L(k)·φ^k - 1 = L(k)·s - 1\n";
    std::cout << "   → Ang relinearization ay NATURAL!\n";
    std::cout << "   → Hindi kailangan ng separate relinearization key!\n\n";

    std::cout << "2. ANG RLWE MULTIPLICATION:\n";
    std::cout << "   (a0, a1) · (b0, b1) = (a0·b0, a0·b1 + a1·b0, a1·b1)\n";
    std::cout << "   Result ay degree-2 ciphertext\n";
    std::cout << "   Relinearize: gamit ang s² = L(k)·s - 1\n";
    std::cout << "   c0_new = a0·b0 + a1·b1·(-1)\n";
    std::cout << "   c1_new = a0·b1 + a1·b0 + a1·b1·L(k)\n\n";

    // RLWE setup na may scalar secret s = φ^k
    NTL::ZZ_pX sk;
    NTL::SetCoeff(sk, 0, NTL::to_ZZ_p(phi_k));

    NTL::ZZ_pX a_poly;
    for (int i = 0; i < N; i++) {
        long a_val = (rng() % 3) - 1;
        NTL::SetCoeff(a_poly, i, NTL::to_ZZ_p(a_val));
    }
    NTL::ZZ_pX e_poly;  // zero noise

    NTL::ZZ_pX pk0 = -(a_poly * sk + e_poly);
    NTL::ZZ_pX pk1 = a_poly;

    std::cout << "3. ENCRYPT AT DECRYPT:\n";
    
    auto encrypt = [&](NTL::ZZ msg) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, NTL::to_ZZ_p(msg));
        NTL::ZZ_pX u;
        for (int i = 0; i < N; i++) {
            NTL::SetCoeff(u, i, NTL::to_ZZ_p((rng() % 3) - 1));
        }
        NTL::ZZ_pX c0 = pk0 * u + m;
        NTL::ZZ_pX c1 = pk1 * u;
        return std::make_pair(c0, c1);
    };

    auto decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * sk;
        // Reduce
        if (NTL::deg(noise) >= N) {
            NTL::ZZ_pX reduced;
            reduced.SetLength(N);
            for (int i = 0; i <= NTL::deg(noise); i++) {
                int rd = i % (2*N);
                if (rd >= N) rd -= N;
                NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(noise, i));
            }
            noise = reduced;
        }
        return NTL::rep(NTL::coeff(noise, 0));
    };

    auto E_Lk = encrypt(L_k);
    auto E_inv_Lk = encrypt(inv_L_k);

    std::cout << "   Decrypt(E_Lk) = " << decrypt(E_Lk) << " (exp " << L_k << ")\n";
    std::cout << "   Decrypt(E_inv_Lk) = " << decrypt(E_inv_Lk) << " (exp " << inv_L_k << ")\n\n";

    // HOMOMORPHIC MULTIPLICATION WITH φ-RELINEARIZATION
    std::cout << "4. HOMOMORPHIC MULTIPLICATION:\n";
    
    auto hom_mult = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a, 
                        std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        
        // φ-natural relinearization
        // c0 = t0 + t2·(-1) = t0 - t2
        // c1 = t1 + t2·L(k)
        NTL::ZZ_pX c0 = t0 - t2;
        NTL::ZZ_pX c1 = t1 + t2 * NTL::to_ZZ_p(L_k);
        
        return std::make_pair(c0, c1);
    };

    // Test: E(1) · E(1) = E(1)
    auto E_one = encrypt(NTL::to_ZZ(1));
    auto E_one_sq = hom_mult(E_one, E_one);
    std::cout << "   Decrypt(E(1)·E(1)) = " << decrypt(E_one_sq) << " (exp 1)\n";

    // Test: E(Lk) · E(inv_Lk) = E(1)
    auto E_prod = hom_mult(E_Lk, E_inv_Lk);
    std::cout << "   Decrypt(E_Lk·E_inv_Lk) = " << decrypt(E_prod) << " (exp 1)\n";

    return 0;
}
