// NAND WITH ENCRYPTED φ^k
// NAND_E(a,b) = E(φ^k) - a·b·E(φ^k)^(-1)

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "NAND WITH ENCRYPTED φ^k\n";
    std::cout << "=======================\n\n";

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

    std::cout << "1. ANG SETUP:\n";
    std::cout << "   E(φ^k) = φ^k + e_bk·ψ^k\n";
    std::cout << "   NAND_E(a,b) = E(φ^k) - a·b·E(φ^k)^(-1)\n\n";

    std::cout << "2. ANG PAG-ANALISA:\n";
    std::cout << "   E(φ^k) = φ^k(1 + e_bk·ψ^k/φ^k)\n";
    std::cout << "   = φ^k(1 + e_bk·ψ^k·φ^(-k))\n";
    std::cout << "   = φ^k(1 + e_bk) (dahil ψ^k·φ^(-k) = 1)\n\n";

    std::cout << "3. ANG KEY INSIGHT:\n";
    std::cout << "   E(φ^k) = φ^k(1 + e_bk) — MULTIPLICATIVE!\n";
    std::cout << "   E(φ^k)^(-1) = φ^(-k)·(1+e_bk)^(-1)\n";
    std::cout << "   → Ang e_bk ay CONSTANT (hindi ψ-dependent)!\n\n";

    std::cout << "4. NAND_E ANALYSIS:\n";
    std::cout << "   NAND_E = φ^k(1+e_bk) - a·b·φ^(-k)·(1+e_bk)^(-1)\n";
    std::cout << "   Kung a = inner_a + e_a·ψ^k\n";
    std::cout << "   Kung b = inner_b + e_b·ψ^k\n";
    std::cout << "   → Kailangan ng buong expansion\n\n";

    // TEST: Simple NAND with E(φ^k)
    NTL::ZZ e_bk = NTL::to_ZZ(5);
    NTL::ZZ E_phi_k = (phi_k + e_bk * psi_k) % Q;
    if (E_phi_k < 0) E_phi_k += Q;
    NTL::ZZ inv_E_phi_k = NTL::InvMod(E_phi_k, Q);

    auto inner_nand_encrypted = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_E_phi_k) % Q;
        NTL::ZZ result = (E_phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    // Test with plain inner values
    std::cout << "5. TEST (simplified — walang ψ-noise sa ct):\n";
    NTL::ZZ inner_0 = NTL::to_ZZ(0);
    NTL::ZZ inner_1 = phi_k;

    auto nand_00 = inner_nand_encrypted(inner_0, inner_0);
    auto nand_01 = inner_nand_encrypted(inner_0, inner_1);
    auto nand_10 = inner_nand_encrypted(inner_1, inner_0);
    auto nand_11 = inner_nand_encrypted(inner_1, inner_1);

    // Decrypt: multiply by φ^(-k)
    auto decrypt_result = [&](NTL::ZZ val) {
        NTL::ZZ scaled = (val * inv_phi_k) % Q;
        NTL::ZZ d_phi = (scaled > phi_k) ? scaled - phi_k : phi_k - scaled;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_phi < d_0;
    };

    std::cout << "   NAND_E(0,0) = " << decrypt_result(nand_00) << " (exp 1)\n";
    std::cout << "   NAND_E(0,1) = " << decrypt_result(nand_01) << " (exp 1)\n";
    std::cout << "   NAND_E(1,0) = " << decrypt_result(nand_10) << " (exp 1)\n";
    std::cout << "   NAND_E(1,1) = " << decrypt_result(nand_11) << " (exp 0)\n";

    return 0;
}
