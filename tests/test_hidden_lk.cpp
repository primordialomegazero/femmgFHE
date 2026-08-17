// HIDDEN L(k) — Paraan para i-hide ang L(k) nang hindi nasisira ang NAND

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "HIDDEN L(k) RESEARCH\n";
    std::cout << "===================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
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

    std::mt19937_64 rng(42);

    std::cout << "1. IDEA 1: BLURRED L(k) NA MAY ENCRYPTED CORRECTION\n";
    std::cout << "   Public: L_blur = L(k) + r·Q\n";
    std::cout << "   Evaluator: kailangan ng L(k) para sa NAND\n";
    std::cout << "   → Kailangan ng homomorphic correction\n\n";

    std::cout << "2. IDEA 2: DUAL L(k) PAIR\n";
    std::cout << "   Public: L_a = L(k) + e_a, L_b = L(k) + e_b\n";
    std::cout << "   NAND gamit ang L_a at L_b — may cancellation ba?\n";
    std::cout << "   → Subukan natin!\n\n";

    // TEST: Dual L(k) na may kaibahan
    NTL::ZZ e_a = NTL::to_ZZ(3);
    NTL::ZZ e_b = NTL::to_ZZ(5);
    NTL::ZZ L_a = L_k + e_a;
    NTL::ZZ L_b = L_k + e_b;
    NTL::ZZ inv_L_a = NTL::InvMod(L_a % Q, Q);

    auto nand_dual = [&](NTL::ZZ a, NTL::ZZ b) {
        // Subukan: NAND = L_a - a·b·inv_L_a
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_L_a) % Q;
        NTL::ZZ result = (L_a - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    auto decrypt_dual = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        // Subukan i-recover mula sa L_a
        NTL::ZZ d_La = (v > L_a) ? v - L_a : L_a - v;
        if (d_La > Q/2) d_La = Q - d_La;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_La < d_0;
    };

    auto ct0 = NTL::to_ZZ(0) + NTL::RandomBnd(Q) * Q;
    auto ct1 = L_k + NTL::RandomBnd(Q) * Q;

    std::cout << "   TEST DUAL L(k):\n";
    std::cout << "   Decrypt(0) = " << decrypt_dual(ct0) << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << decrypt_dual(ct1) << " (exp 1)\n\n";

    std::cout << "3. IDEA 3: HOMOMORPHIC CORRECTION\n";
    std::cout << "   Public: L_blur = L(k) + r·Q\n";
    std::cout << "   Correction key: CK = E(r) — encrypted r\n";
    std::cout << "   Evaluator: L(k) = L_blur - CK·Q (homomorphically)\n";
    std::cout << "   → Pero Q ay public, kaya CK = E(r) ay nabubuksan\n\n";

    std::cout << "4. IDEA 4: MATRIX-BASED HIDING\n";
    std::cout << "   Public: 2×2 matrix M = [[L(k), r], [s, t]]\n";
    std::cout << "   Determinant: L(k)·t - r·s = 1 (trapdoor)\n";
    std::cout << "   → Kailangan i-factor ang matrix para sa L(k)\n";
    std::cout << "   → Kung random ang r, s, t, mahirap i-factor\n\n";

    std::cout << "5. ANG PINAKA-PROMISING:\n";
    std::cout << "   Public: L_blur = L(k) + r·Q\n";
    std::cout << "   Ang NAND ay kailangan ng L(k) mismo\n";
    std::cout << "   → Kailangan ng FULLY HOMOMORPHIC na paraan\n";
    std::cout << "     para i-recover ang L(k) mula sa L_blur\n";
    std::cout << "   → Ito ay BOOTSTRAPPING!\n\n";

    std::cout << "6. ANG NATURAL NA SAGOT:\n";
    std::cout << "   Dahil L(k) ay INTEGER at UNIVERSAL,\n";
    std::cout << "   pwede nating i-encrypt ito sa mas malaking Q:\n";
    std::cout << "   L_encrypted = L(k) + r·Q_big\n";
    std::cout << "   Ang evaluator ay may L_encrypted\n";
    std::cout << "   Ang NAND ay gumagana kung ang L_encrypted\n";
    std::cout << "   ay ma-reduce sa L(k) homomorphically\n\n";

    std::cout << "7. ANG KEY INSIGHT:\n";
    std::cout << "   Kung ang r·Q_big ay ma-reduce modulo Q_small,\n";
    std::cout << "   ang L(k) ay ma-recover nang walang decrypt!\n";
    std::cout << "   → Ito ay NATURAL MODULUS SWITCHING!\n";

    return 0;
}
