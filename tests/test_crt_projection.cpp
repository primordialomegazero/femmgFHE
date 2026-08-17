// CRT PROJECTION — I-project sa φ o ψ direction
#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "CRT PROJECTION RESEARCH\n";
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

    std::cout << "1. CRT DECOMPOSITION:\n";
    std::cout << "   Z_Q[φ] ≅ Z_Q × Z_Q\n";
    std::cout << "   Element v ay may (v_φ, v_ψ) components\n\n";

    std::cout << "2. PROJECTION FORMULA:\n";
    std::cout << "   v_φ = v·ψ^k·(φ^k-ψ^k)^(-1)\n";
    std::cout << "   v_ψ = v·φ^k·(ψ^k-φ^k)^(-1)\n";
    std::cout << "   (Ito ang standard CRT projection)\n\n";

    std::cout << "3. ANG TAMANG PROJECTION PARA SA FHE:\n";
    std::cout << "   Message ay nasa φ-direction\n";
    std::cout << "   Noise ay nasa ψ-direction\n";
    std::cout << "   → Project sa φ para makuha ang message\n";
    std::cout << "   → Project sa ψ para makuha ang noise\n\n";

    // TEST: Projection
    NTL::ZZ inner = phi_k;  // message 1
    NTL::ZZ e = NTL::to_ZZ(10);
    NTL::ZZ ct = inner + e * psi_k;

    // Project sa φ-direction: multiply by ψ^k
    NTL::ZZ proj_phi = (ct * psi_k) % Q;
    std::cout << "4. PROJECTION TEST:\n";
    std::cout << "   ct = φ^k + 10·ψ^k\n";
    std::cout << "   ct·ψ^k = φ^k·ψ^k + 10·ψ^(2k)\n";
    std::cout << "          = 1 + 10·ψ^(2k)\n";
    std::cout << "   → Ang 1 ay maliit, ang 10·ψ^(2k) ay malaki\n";
    std::cout << "   → HINDI ito nag-i-isolate ng message\n\n";

    // Project sa ψ-direction: multiply by φ^k
    NTL::ZZ proj_psi = (ct * phi_k) % Q;
    std::cout << "   ct·φ^k = φ^(2k) + 10·φ^k·ψ^k\n";
    std::cout << "          = φ^(2k) + 10\n";
    std::cout << "   → φ^(2k) ay malaki, 10 ay maliit\n";
    std::cout << "   → ITO AY MAS MAGANDA!\n\n";

    // I-verify ang projection
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
    std::cout << "5. VERIFICATION:\n";
    std::cout << "   φ^(2k) = " << phi_2k << "\n";
    std::cout << "   ct·φ^k = " << proj_psi << "\n";
    std::cout << "   difference = " << (proj_psi - phi_2k + Q) % Q << " (dapat 10)\n";

    return 0;
}
