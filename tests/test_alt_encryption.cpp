// ALTERNATIVE φ^k ENCRYPTION METHODS
// Hanapin ang tamang paraan para ma-encrypt ang φ^k

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "ALTERNATIVE φ^k ENCRYPTION METHODS\n";
    std::cout << "=================================\n\n";

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

    std::mt19937_64 rng(42);

    std::cout << "METHOD 1: ADDITIVE (current)\n";
    std::cout << "   E(φ^k) = φ^k + e·ψ^k\n";
    std::cout << "   Problem: May multiplicative factor (1+e)\n\n";

    std::cout << "METHOD 2: MULTIPLICATIVE\n";
    std::cout << "   E(φ^k) = φ^k · (1 + e·ψ^k)\n";
    std::cout << "   = φ^k + e·φ^k·ψ^k = φ^k + e\n";
    std::cout << "   → Ang e ay CONSTANT (hindi ψ-dependent)!\n";
    std::cout << "   → E(φ^k) = φ^k + e — SOBRANG SIMPLE!\n\n";

    // TEST METHOD 2
    std::cout << "TEST METHOD 2: E(φ^k) = φ^k + e\n";
    NTL::ZZ e_bk = NTL::to_ZZ(5);
    NTL::ZZ E_phi_k = (phi_k + e_bk) % Q;
    NTL::ZZ inv_E_phi_k = NTL::InvMod(E_phi_k, Q);

    auto nand_m2 = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_E_phi_k) % Q;
        NTL::ZZ result = (E_phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    auto decrypt_m2 = [&](NTL::ZZ val) {
        // val = NAND result
        // Decrypt: val - e_bk (since E(φ^k) = φ^k + e_bk)
        NTL::ZZ adjusted = (val - e_bk + Q) % Q;
        NTL::ZZ scaled = (adjusted * inv_phi_k) % Q;
        NTL::ZZ d_phi = (scaled > phi_k) ? scaled - phi_k : phi_k - scaled;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_phi < d_0;
    };

    NTL::ZZ inner_0 = NTL::to_ZZ(0);
    NTL::ZZ inner_1 = phi_k;

    std::cout << "   NAND(0,0) = " << decrypt_m2(nand_m2(inner_0, inner_0)) << " (exp 1)\n";
    std::cout << "   NAND(0,1) = " << decrypt_m2(nand_m2(inner_0, inner_1)) << " (exp 1)\n";
    std::cout << "   NAND(1,0) = " << decrypt_m2(nand_m2(inner_1, inner_0)) << " (exp 1)\n";
    std::cout << "   NAND(1,1) = " << decrypt_m2(nand_m2(inner_1, inner_1)) << " (exp 0)\n\n";

    std::cout << "METHOD 3: CONJUGATE PAIR\n";
    std::cout << "   E(φ^k) = (φ^k + e·ψ^k) · (φ^k - e·ψ^k)\n";
    std::cout << "   = φ^(2k) - e²·ψ^(2k)\n";
    std::cout << "   → Mas complex pero baka may cancellation\n\n";

    std::cout << "METHOD 4: LUCAS-EMBEDDED\n";
    std::cout << "   E(φ^k) = L(k) - ψ^k\n";
    std::cout << "   = φ^k (EXACT!)\n";
    std::cout << "   → Pero ψ^k ay secret!\n\n";

    std::cout << "METHOD 5: TWISTED ADDITIVE\n";
    std::cout << "   E(φ^k) = φ^k + e·(φ^k + ψ^k)\n";
    std::cout << "   = φ^k(1+e) + e·ψ^k\n";
    std::cout << "   = φ^k(1+e) + e·ψ^k\n";
    std::cout << "   → May φ^k(1+e) factor pa rin\n\n";

    std::cout << "METHOD 6: SELF-NORMALIZING\n";
    std::cout << "   E(φ^k) = φ^k · L(k) · inv_phi_k\n";
    std::cout << "   = L(k) — CONSTANT!\n";
    std::cout << "   → HINDI ito encrypted φ^k — L(k) lamang!\n";

    return 0;
}
