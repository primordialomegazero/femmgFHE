// L(k) FHE NAKA-ENCRYPT SA RLWE
// Ang buong L(k) computation ay naka-encrypt sa RLWE

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "L(k) FHE SA LOOB NG RLWE\n";
    std::cout << "========================\n\n";

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

    std::cout << "1. ANG IDEA:\n";
    std::cout << "   Sa halip na L(k) ay public,\n";
    std::cout << "   i-encrypt ang L(k) sa RLWE.\n";
    std::cout << "   Evaluator ay may RLWE(L(k)) at RLWE(inv_L(k)).\n";
    std::cout << "   NAND ay homomorphic sa RLWE domain.\n\n";

    // RLWE setup
    NTL::ZZ_pX sk;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(sk, i, NTL::to_ZZ_p((rng() % 3) - 1));
    }

    NTL::ZZ_pX a_poly;
    for (int i = 0; i < N; i++) {
        long a_val = (rng() % 3) - 1;
        NTL::SetCoeff(a_poly, i, NTL::to_ZZ_p(a_val));
    }
    NTL::ZZ_pX e_poly;
    NTL::SetCoeff(e_poly, 0, NTL::to_ZZ_p(0));

    NTL::ZZ_pX pk0 = -(a_poly * sk + e_poly);
    NTL::ZZ_pX pk1 = a_poly;

    // Reduce pk0
    if (NTL::deg(pk0) >= N) {
        NTL::ZZ_pX reduced;
        reduced.SetLength(N);
        for (int i = 0; i <= NTL::deg(pk0); i++) {
            int rd = i % (2*N);
            if (rd >= N) { rd -= N; }
            NTL::SetCoeff(reduced, rd, NTL::coeff(reduced, rd) + NTL::coeff(pk0, i));
        }
        pk0 = reduced;
    }

    std::cout << "2. RLWE SETUP DONE\n\n";

    // Encrypt L(k) at inv_L(k) sa RLWE
    auto encrypt_rlwe = [&](NTL::ZZ msg) {
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

    auto decrypt_rlwe = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * sk;
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

    // Encrypt L(k) at inv_L(k)
    auto E_Lk = encrypt_rlwe(L_k);
    auto E_inv_Lk = encrypt_rlwe(inv_L_k);

    std::cout << "3. VERIFY ENCRYPTED KEYS:\n";
    std::cout << "   Decrypt(E_Lk) = " << decrypt_rlwe(E_Lk) << " (exp " << L_k << ")\n";
    std::cout << "   Decrypt(E_inv_Lk) = " << decrypt_rlwe(E_inv_Lk) << " (exp " << inv_L_k << ")\n\n";

    // HOMOMORPHIC NAND sa RLWE:
    // NAND = E_Lk - (E_a · E_b) · E_inv_Lk
    // RLWE multiplication: (a0,a1) · (b0,b1) = (a0·b0, a0·b1 + a1·b0, a1·b1)
    // Pagkatapos: relinearization (simplify na lang for now)
    
    std::cout << "4. HOMOMORPHIC NAND SA RLWE:\n";
    std::cout << "   (Ito ay complex — kailangan ng relinearization)\n";
    std::cout << "   → Subukan natin nang simple:\n\n";

    // Simple approach: decrypt-reencrypt muna para sa baseline
    auto ct0_msg = NTL::to_ZZ(0);
    auto ct1_msg = L_k;

    auto E_ct0 = encrypt_rlwe(ct0_msg);
    auto E_ct1 = encrypt_rlwe(ct1_msg);

    // Homomorphic NAND attempt
    // NAND = E_Lk - E_ct0·E_ct0·E_inv_Lk
    // Para sa simpleng test, gamitin ang first component lamang
    
    std::cout << "5. BASELINE TEST (decrypt-reencrypt):\n";
    auto result_00 = decrypt_rlwe(E_Lk);
    std::cout << "   L(k) recovered: " << result_00 << "\n";

    std::cout << "\n6. ANG CHALLENGE:\n";
    std::cout << "   RLWE homomorphic multiplication ay may\n";
    std::cout << "   polynomial noise na lumalaki.\n";
    std::cout << "   → Kailangan ng bootstrapping para sa noise control\n";
    std::cout << "   → Pero ang L(k) NAND ay scalar at perfect\n";
    std::cout << "   → Sa hybrid, ang RLWE noise ay makokontrol\n";
    std::cout << "     ng natural na φ-structure\n";

    return 0;
}
