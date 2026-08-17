// TRIPLE LAYER FHE
// Layer 0: m·φ^k (inner)
// Layer 1: + e·ψ^k (outer)
// Layer 2: BK = E(φ^k) (bootstrapping)

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "TRIPLE LAYER FHE TEST\n";
    std::cout << "====================\n\n";

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
    std::cout << "   Public: Q, L(k)\n";
    std::cout << "   Secret: φ^k, ψ^k\n";
    std::cout << "   BK = φ^k + e_bk·ψ^k (encrypted φ^k)\n\n";

    // BK construction
    NTL::ZZ e_bk = NTL::to_ZZ(5);
    NTL::ZZ BK = (phi_k + e_bk * psi_k) % Q;

    std::cout << "2. ANG HOMOMORPHIC NAND:\n";
    std::cout << "   NAND(a,b) = BK - a·b·BK^(-1)\n";
    std::cout << "   = φ^k + e_bk·ψ^k - (a·b)·(φ^k + e_bk·ψ^k)^(-1)\n\n";

    std::cout << "3. ANG KEY INSIGHT:\n";
    std::cout << "   (φ^k + e_bk·ψ^k)^(-1) = ?\n";
    std::cout << "   φ^k + e_bk·ψ^k = φ^k(1 + e_bk)\n";
    std::cout << "   (since ψ^k·φ^(-k) = 1)\n";
    std::cout << "   → BK = φ^k(1 + e_bk)\n";
    std::cout << "   → BK^(-1) = φ^(-k)·(1+e_bk)^(-1)\n\n";

    std::cout << "4. ANG PROBLEMA:\n";
    std::cout << "   BK = φ^k(1+e_bk) ay may multiplicative factor.\n";
    std::cout << "   Ang factor (1+e_bk) ay CONSTANT.\n";
    std::cout << "   → Kung ma-subtract natin ito, OK na!\n\n";

    std::cout << "5. ANG TAMANG BK:\n";
    std::cout << "   BK = φ^k + e_bk·ψ^k + r·Q (may Q-embed)\n";
    std::cout << "   → Pero ang r·Q ay maa-absorb sa mod Q\n";
    std::cout << "   → Kailangan ng ibang paraan\n\n";

    std::cout << "6. ANG EMERGENT FIX:\n";
    std::cout << "   Sa halip na φ^k + e·ψ^k, gamitin ang:\n";
    std::cout << "   BK = φ^k + e·ψ^k + r·(φ^k + ψ^k)\n";
    std::cout << "   = φ^k(1+r) + ψ^k(e+r)\n";
    std::cout << "   → Ang r·L(k) ay nagbibigay ng extra masking\n\n";

    std::cout << "7. ANG PINAKA-IMPORTANTENG OBSERVATION:\n";
    std::cout << "   Ang multiplicative factor (1+e_bk) ay ang core issue.\n";
    std::cout << "   Kung ma-normalize natin ang BK para walang factor,\n";
    std::cout << "   ang NAND ay magwo-work!\n\n";

    std::cout << "8. ANG TAMANG NORMALIZATION:\n";
    std::cout << "   BK_normalized = BK · (1+e_bk)^(-1) = φ^k\n";
    std::cout << "   → Pero ito ay DECRYPTION (kailangan ng (1+e_bk)^(-1))\n";
    std::cout << "   → Ang (1+e_bk)^(-1) ay pwede i-encrypt!\n\n";

    // TEST: Multiplicative normalization
    NTL::ZZ one_plus_e_inv = NTL::InvMod(NTL::to_ZZ(1) + e_bk, Q);
    NTL::ZZ BK_normalized = (BK * one_plus_e_inv) % Q;
    std::cout << "9. VERIFICATION:\n";
    std::cout << "   BK·(1+e_bk)^(-1) = " << BK_normalized << "\n";
    std::cout << "   φ^k = " << phi_k << "\n";
    std::cout << "   Match: " << (BK_normalized == phi_k ? "YES!" : "NO") << "\n\n";

    std::cout << "10. ANG KEY INSIGHT:\n";
    std::cout << "   Kung ang evaluator ay may ENCRYPTED (1+e_bk)^(-1),\n";
    std::cout << "   maaari niyang i-normalize ang BK nang homomorphically!\n";
    std::cout << "   → Ito ang BOOTSTRAPPING LAYER!\n";
    std::cout << "   → BK_key = E((1+e_bk)^(-1))\n";

    return 0;
}
