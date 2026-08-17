// ψ^k NOISE CANCELLATION SA NAND
// Hanapin kung may natural na paraan para ma-cancel ang ψ^k terms

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "ψ^k NOISE CANCELLATION SA NAND\n";
    std::cout << "==============================\n\n";

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

    std::cout << "1. ANG SETUP:\n";
    std::cout << "   ct = m·L(k) + e·ψ^k + r·Q\n\n";

    std::cout << "2. ANG NAND FORMULA:\n";
    std::cout << "   NAND(a,b) = L(k) - a·b·inv_L(k)\n\n";

    std::cout << "3. ANG EXPANSION:\n";
    std::cout << "   a = m_a·L(k) + e_a·ψ^k\n";
    std::cout << "   b = m_b·L(k) + e_b·ψ^k\n\n";
    std::cout << "   a·b = m_a·m_b·L(k)² + (m_a·e_b + e_a·m_b)·L(k)·ψ^k + e_a·e_b·ψ^(2k)\n\n";

    std::cout << "4. ANG KEY QUESTION:\n";
    std::cout << "   May natural ba na cancellation para sa:\n";
    std::cout << "   (m_a·e_b + e_a·m_b)·L(k)·ψ^k + e_a·e_b·ψ^(2k)?\n\n";

    // Subukan ang iba't ibang NAND formula
    std::cout << "5. SUBUKAN: NAND na may ψ^k correction\n";
    std::cout << "   NAND_corrected = NAND - correction·ψ^k\n";
    std::cout << "   → Ano ang tamang correction?\n\n";

    // Test: direct NAND na may noise
    std::mt19937_64 rng(42);
    
    auto encrypt = [&](bool bit, NTL::ZZ e_val) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ m_val = bit ? L_k : NTL::to_ZZ(0);
        return m_val + e_val * psi_k + r * Q;
    };

    auto nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        if (a_mod < 0) a_mod += Q;
        NTL::ZZ b_mod = b % Q;
        if (b_mod < 0) b_mod += Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_L_k) % Q;
        NTL::ZZ result = (L_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ d_L = (v > L_k) ? v - L_k : L_k - v;
        if (d_L > Q/2) d_L = Q - d_L;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_L < d_0;
    };

    // Test with small noise
    std::cout << "6. TEST WITH SMALL NOISE (e=1):\n";
    auto ct0 = encrypt(false, NTL::to_ZZ(1));
    auto ct1 = encrypt(true, NTL::to_ZZ(1));

    std::cout << "   Decrypt(0) = " << decrypt(ct0) << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << decrypt(ct1) << " (exp 1)\n";
    std::cout << "   NAND(0,0) = " << decrypt(nand(ct0, ct0)) << " (exp 1)\n";
    std::cout << "   NAND(0,1) = " << decrypt(nand(ct0, ct1)) << " (exp 1)\n";
    std::cout << "   NAND(1,0) = " << decrypt(nand(ct1, ct0)) << " (exp 1)\n";
    std::cout << "   NAND(1,1) = " << decrypt(nand(ct1, ct1)) << " (exp 0)\n\n";

    std::cout << "7. DEEP CHAIN WITH SMALL NOISE:\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand(current, current);
    }
    std::cout << "   Errors: " << errors << "/101\n\n";

    std::cout << "8. KEY INSIGHT:\n";
    std::cout << "   Ang ψ^k noise ay may e·ψ^(2k) term sa NAND.\n";
    std::cout << "   Kung e ay maliit, ang noise ay manageable.\n";
    std::cout << "   → May OPTIMAL e na nagbibigay ng:\n";
    std::cout << "     - Enough security (GCD hidden)\n";
    std::cout << "     - Enough correctness (NAND works)\n";

    return 0;
}
