// RECURSIVE HOMOMORPHIC DECRYPTION
// Multi-layer na encrypted decryption

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "RECURSIVE HOMOMORPHIC DECRYPTION\n";
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
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);

    std::mt19937_64 rng(42);

    std::cout << "1. ANG CONCEPT:\n";
    std::cout << "   Layer 0: message m\n";
    std::cout << "   Layer 1: m·φ^k (inner FHE)\n";
    std::cout << "   Layer 2: + e·ψ^k (outer FHE)\n";
    std::cout << "   Layer 3: + e2·ψ^k (encrypted decryption key)\n\n";

    std::cout << "2. ANG TAMANG RECURSIVE STRUCTURE:\n";
    std::cout << "   BK_L2 = φ^k + e_bk·ψ^k (encrypted φ^k)\n";
    std::cout << "   BK_L3 = BK_L2 + e_bk2·ψ^k (double encrypted)\n";
    std::cout << "   → Evaluator ay may BK_L3 lamang\n";
    std::cout << "   → Hindi makakakita ng φ^k o BK_L2\n\n";

    // SIMPLIFIED TEST: Can the evaluator recover message using BK_L2?
    std::cout << "3. TEST: Evaluator ay may BK_L2 = φ^k + e_bk·ψ^k\n";
    NTL::ZZ e_bk = NTL::to_ZZ(5);
    NTL::ZZ BK_L2 = phi_k + e_bk * psi_k;

    NTL::ZZ inner = phi_k;  // message 1
    NTL::ZZ e_ct = NTL::to_ZZ(7);
    NTL::ZZ ct = inner + e_ct * psi_k;

    // Evaluator: ct·BK_L2
    NTL::ZZ eval = (ct * BK_L2) % Q;
    NTL::ZZ scaled = (eval * inv_phi_k) % Q;

    std::cout << "   ct·BK_L2·φ^(-k) = " << scaled << "\n";
    std::cout << "   Expected: φ^k + noise\n\n";

    // KEY OBSERVATION:
    std::cout << "4. CROSS TERMS ANALYSIS:\n";
    std::cout << "   ct·BK_L2 = (φ^k + 7ψ^k)(φ^k + 5ψ^k)\n";
    std::cout << "   = φ^(2k) + 5·ψ^k·φ^k + 7·ψ^k·φ^k + 35·ψ^(2k)\n";
    std::cout << "   = φ^(2k) + 12 + 35·ψ^(2k)\n\n";
    std::cout << "   Ang 12 ay CONSTANT\n";
    std::cout << "   Ang 35·ψ^(2k) ay MALAKING noise\n\n";

    std::cout << "5. ANG PROBLEM:\n";
    std::cout << "   ψ^(2k) = L(k)·ψ^k - 1 (Lucas)\n";
    std::cout << "   35·ψ^(2k) = 35·L(k)·ψ^k - 35\n";
    std::cout << "   → May MALAKING ψ^k term pa rin\n";
    std::cout << "   → Hindi ma-cancel nang simple\n\n";

    std::cout << "6. ANG RECURSIVE SOLUTION:\n";
    std::cout << "   Kung may BK_L3 = BK_L2 + e2·ψ^k\n";
    std::cout << "   at BK_L4 = BK_L3 + e3·ψ^k, etc.\n";
    std::cout << "   Ang bawat layer ay nagbababa ng noise.\n";
    std::cout << "   → Kailangan ng tamang depth!\n";

    return 0;
}
