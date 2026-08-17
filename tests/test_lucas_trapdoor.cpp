// LUCAS TRAPDOOR — L(k) bilang public key
// Public: Q, L(k) = φ^k + ψ^k
// Secret: φ^k

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "LUCAS TRAPDOOR TEST\n";
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
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);

    NTL::ZZ L_k = (phi_k + psi_k) % Q;

    std::mt19937_64 rng(42);

    std::cout << "1. PUBLIC KEY:\n";
    std::cout << "   L(k) = φ^k + ψ^k = " << L_k << "\n\n";

    std::cout << "2. THE PROBLEM:\n";
    std::cout << "   ct = m·φ^k + e·ψ^k\n";
    std::cout << "   Kailangan: ct·(φ^k projection) para ma-recover ang m\n";
    std::cout << "   Pero φ^k ay SECRET\n\n";

    std::cout << "3. SUBUKAN: ct·L(k)\n";
    NTL::ZZ inner = phi_k;  // message 1
    NTL::ZZ e = NTL::to_ZZ(5);
    NTL::ZZ ct = inner + e * psi_k;
    
    NTL::ZZ proj_L = (ct * L_k) % Q;
    std::cout << "   ct·L(k) = (φ^k + 5ψ^k)·(φ^k + ψ^k)\n";
    std::cout << "   = φ^(2k) + φ^k·ψ^k + 5ψ^k·φ^k + 5ψ^(2k)\n";
    std::cout << "   = φ^(2k) + 1 + 5 + 5ψ^(2k)\n";
    std::cout << "   = φ^(2k) + 6 + 5ψ^(2k)\n\n";
    std::cout << "   Result: φ^(2k) + 6 + 5ψ^(2k)\n\n";

    std::cout << "4. ANG PROBLEM:\n";
    std::cout << "   Ang φ^(2k) ay malaki (dominant)\n";
    std::cout << "   Ang 5ψ^(2k) ay malaki rin (noise)\n";
    std::cout << "   → HINDI pa malinaw ang separation\n\n";

    std::cout << "5. TAMANG PROJECTION:\n";
    std::cout << "   Kailangan: ct·φ^k (hindi ct·L(k))\n";
    std::cout << "   ct·φ^k = φ^(2k) + 5ψ^k·φ^k = φ^(2k) + 5\n";
    std::cout << "   → PERFECT! 5 lang ang noise\n";
    std::cout << "   → Pero φ^k ay SECRET!\n\n";

    std::cout << "6. ANG LUCAS TRAPDOOR CANDIDATE:\n";
    std::cout << "   L(k) = φ^k + ψ^k\n";
    std::cout << "   L(k) - ψ^k = φ^k (pero ψ^k ay secret din!)\n";
    std::cout << "   L(k) - φ^k = ψ^k\n\n";

    std::cout << "7. ANG KEY QUESTION:\n";
    std::cout << "   May algebraic ba na paraan para sa evaluator\n";
    std::cout << "   na mag-project sa φ^k gamit L(k) lamang?\n\n";

    // I-check kung may simpleng relasyon
    std::cout << "8. ATTEMPT: ct·(L(k) - ?)\n";
    std::cout << "   Kung alam natin ang ψ^k, pwede tayong mag:\n";
    std::cout << "   ct·(L(k) - ψ^k) = ct·φ^k = φ^(2k) + 5\n";
    std::cout << "   → PERFECT!\n";
    std::cout << "   Pero ψ^k ay secret din!\n\n";

    std::cout << "9. ANG PARADOX:\n";
    std::cout << "   L(k) = φ^k + ψ^k ay PUBLIC\n";
    std::cout << "   φ^k ay SECRET\n";
    std::cout << "   ψ^k ay SECRET\n";
    std::cout << "   Ang evaluator ay may L(k) lamang\n";
    std::cout << "   → Hindi ma-recover ang φ^k o ψ^k individually\n";
    std::cout << "   → Kailangan ng ibang approach\n";

    return 0;
}
