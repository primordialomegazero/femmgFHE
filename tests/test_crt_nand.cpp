// CRT PROJECTION HOMOMORPHIC NAND
// π_φ para sa message, π_ψ para sa noise

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "CRT PROJECTION HOMOMORPHIC NAND\n";
    std::cout << "================================\n\n";

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
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);

    std::cout << "1. ANG CRT PROJECTION KEYS:\n";
    std::cout << "   π_φ ay kayang i-compute gamit ang ψ\n";
    std::cout << "   π_ψ ay kayang i-compute gamit ang φ\n\n";

    std::cout << "2. ANG KEY QUESTION:\n";
    std::cout << "   May public bang projection key na nagbibigay\n";
    std::cout << "   ng φ-projection nang walang φ?\n\n";

    std::cout << "3. ANG LUCAS PROJECTION CANDIDATE:\n";
    std::cout << "   L(k) = φ^k + ψ^k\n";
    std::cout << "   L(2k) = φ^(2k) + ψ^(2k)\n";
    std::cout << "   → May relasyon bang nagbibigay ng φ^k?\n\n";

    // Compute L(2k)
    NTL::ZZ L_2k = (L_k * L_k - 2) % Q;
    if (L_2k < 0) L_2k += Q;

    std::cout << "4. ANG LUCAS SEQUENCE:\n";
    std::cout << "   L(k) = " << L_k << "\n";
    std::cout << "   L(2k) = " << L_2k << "\n\n";

    std::cout << "5. ANG EMERGENT RELASYON:\n";
    std::cout << "   φ^k = (L(k) + (φ^k - ψ^k))/2\n";
    std::cout << "   φ^k = (L(k) + √(L(k)²-4))/2\n";
    std::cout << "   → Kailangan ng √(L(k)²-4) — SECRET!\n\n";

    std::cout << "6. ANG PRACTICAL NA SAGOT:\n";
    std::cout << "   Walang public na paraan para makuha ang φ^k\n";
    std::cout << "   mula sa L(k) nang walang square root.\n";
    std::cout << "   → Kailangan ng ibang approach\n\n";

    std::cout << "7. ANG ENCRYPTED PROJECTION KEY:\n";
    std::cout << "   BK_φ = E(φ^k) na naka-encrypt sa ψ-direction\n";
    std::cout << "   BK_φ = φ^k + e·ψ^k\n";
    std::cout << "   Ang evaluator ay may BK_φ (hindi φ^k mismo)\n";
    std::cout << "   → NAND sa φ-direction gamit ang BK_φ\n\n";

    // TEST: NAND na may BK_φ
    std::mt19937_64 rng(42);
    NTL::ZZ e_bk = NTL::to_ZZ(3);
    NTL::ZZ BK_phi = (phi_k + e_bk * psi_k) % Q;
    NTL::ZZ inv_BK_phi = NTL::InvMod(BK_phi, Q);

    auto hom_nand = [&](NTL::ZZ a, NTL::ZZ b) {
        // NAND = BK_φ - a·b·BK_φ^(-1)
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_BK_phi) % Q;
        NTL::ZZ result = (BK_phi - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    // Test
    NTL::ZZ inner_0 = NTL::to_ZZ(0);
    NTL::ZZ inner_1 = phi_k;

    // Decrypt: multiply by inv_phi_k at i-adjust
    auto decrypt_nand = [&](NTL::ZZ val) {
        // val = BK_φ - a·b·BK_φ^(-1)
        // Adjust: val - e_bk·ψ^k para makuha ang φ^k component
        NTL::ZZ adjusted = (val - e_bk * psi_k + Q) % Q;
        NTL::ZZ scaled = (adjusted * inv_phi_k) % Q;
        NTL::ZZ d_phi = (scaled > phi_k) ? scaled - phi_k : phi_k - scaled;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_phi < d_0;
    };

    std::cout << "8. NAND TEST (simplified):\n";
    std::cout << "   NAND(0,0) = " << decrypt_nand(hom_nand(inner_0, inner_0)) << " (exp 1)\n";
    std::cout << "   NAND(0,1) = " << decrypt_nand(hom_nand(inner_0, inner_1)) << " (exp 1)\n";
    std::cout << "   NAND(1,0) = " << decrypt_nand(hom_nand(inner_1, inner_0)) << " (exp 1)\n";
    std::cout << "   NAND(1,1) = " << decrypt_nand(hom_nand(inner_1, inner_1)) << " (exp 0)\n";

    return 0;
}
