// L(k) BILANG NATURAL BK
// BK = L(k) = φ^k + φ^(-k) = φ^k + ψ^k

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "L(k) BILANG NATURAL BK\n";
    std::cout << "======================\n\n";

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

    std::cout << "1. ANG NATURAL BK:\n";
    std::cout << "   BK = L(k) = φ^k + ψ^k = " << L_k << "\n\n";

    std::cout << "2. ANG NAND GAMIT L(k):\n";
    std::cout << "   NAND(a,b) = L(k) - a·b·L(k)^(-1)\n";
    std::cout << "   = (φ^k + ψ^k) - a·b·(φ^k + ψ^k)^(-1)\n\n";

    std::cout << "3. ANG EMERGENT PROPERTY:\n";
    std::cout << "   L(k) = φ^k + ψ^k = φ^k + φ^(-k)\n";
    std::cout << "   L(k)^(-1) = ?\n";
    std::cout << "   (φ^k + ψ^k)^(-1) = φ^k·ψ^k/(φ^k + ψ^k) = 1/L(k)\n";
    std::cout << "   → L(k)^(-1) ay computable kung public ang L(k)!\n\n";

    // Compute L(k)^(-1)
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);

    std::cout << "4. ANG HOMOMORPHIC NAND:\n";
    std::cout << "   NAND(a,b) = L(k) - a·b·inv_L(k)\n";
    std::cout << "   → Kailangan lang ng L(k) at inv_L(k) — PUBLIC!\n\n";

    // TEST: NAND with L(k)
    auto hom_nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_L_k) % Q;
        NTL::ZZ result = (L_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    // Test with inner values
    NTL::ZZ inner_0 = NTL::to_ZZ(0);
    NTL::ZZ inner_1 = phi_k;

    // Decrypt: ang result ay nasa L(k)-space
    // result = L(k) - a·b·L(k)^(-1)
    // Kung a=0,b=0: result = L(k) — public!
    // Kung a=φ^k,b=φ^k: result = L(k) - φ^(2k)·L(k)^(-1)
    
    auto decrypt_L = [&](NTL::ZZ val) {
        // Para sa simpleng test, i-check lang kung val == L(k)
        NTL::ZZ diff_L = (val > L_k) ? val - L_k : L_k - val;
        NTL::ZZ diff_0 = (val < Q/2) ? val : Q - val;
        return diff_L < diff_0;
    };

    std::cout << "5. NAND TEST (L(k) space):\n";
    std::cout << "   NAND(0,0) = " << decrypt_L(hom_nand(inner_0, inner_0)) << " (exp 1)\n";
    std::cout << "   NAND(0,1) = " << decrypt_L(hom_nand(inner_0, inner_1)) << " (exp 1)\n";
    std::cout << "   NAND(1,0) = " << decrypt_L(hom_nand(inner_1, inner_0)) << " (exp 1)\n";
    std::cout << "   NAND(1,1) = " << decrypt_L(hom_nand(inner_1, inner_1)) << " (exp 0)\n\n";

    // Compute actual values
    std::cout << "6. ACTUAL VALUES:\n";
    auto r00 = hom_nand(inner_0, inner_0);
    auto r01 = hom_nand(inner_0, inner_1);
    auto r10 = hom_nand(inner_1, inner_0);
    auto r11 = hom_nand(inner_1, inner_1);

    std::cout << "   NAND(0,0) = " << r00 << "\n";
    std::cout << "   NAND(0,1) = " << r01 << "\n";
    std::cout << "   NAND(1,0) = " << r10 << "\n";
    std::cout << "   NAND(1,1) = " << r11 << "\n";
    std::cout << "   L(k) = " << L_k << "\n";

    return 0;
}
