// φ-PROJECTION RESEARCH V2 — Correct Modular Handling
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-PROJECTION V2\n";
    std::cout << "  Correct Modular Inverse\n";
    std::cout << "========================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    NTL::ZZ inv_psi_k = NTL::InvMod(psi_k, Q);

    // Tamang Lucas at Fibonacci — BUO bilang integer, hindi mod Q
    // L(42) = 599074578, F(42) = 267914296
    NTL::ZZ L_k = NTL::to_ZZ(599074578);
    NTL::ZZ F_k = NTL::to_ZZ(267914296);

    // Sa modular arithmetic, ang D_k = F_k·√5 mod Q
    NTL::ZZ D_k = (F_k * sqrt5) % Q;

    // I-verify: φ^k = (L_k + D_k)/2 mod Q
    NTL::ZZ check_phi = ((L_k + D_k) % Q * inv2) % Q;
    std::cout << "φ^k check: " << (check_phi == phi_k ? "MATCH" : "NO MATCH") << "\n";

    NTL::ZZ check_psi = ((L_k - D_k + Q) % Q * inv2) % Q;
    std::cout << "ψ^k check: " << (check_psi == psi_k ? "MATCH" : "NO MATCH") << "\n\n";

    // ============================================
    // ANG KEY QUESTION: May orthogonality ba talaga?
    // ============================================
    std::cout << "ORTHOGONALITY TEST:\n";
    std::cout << "====================\n\n";

    // I-verify kung L_k at D_k ay "orthogonal" sa modular sense
    // L_k · D_k mod Q = ?
    NTL::ZZ L_D_product = (NTL::to_ZZ(L_k) % Q) * D_k % Q;
    std::cout << "L_k · D_k mod Q = " << L_D_product << "\n";
    std::cout << "(Dapat 0 kung orthogonal)\n\n";

    // Mas mahalaga: φ^k at ψ^k ay may inner product
    NTL::ZZ phi_psi_product = (phi_k * psi_k) % Q;
    std::cout << "φ^k · ψ^k mod Q = " << phi_psi_product << "\n";
    std::cout << "(Dapat 1 o -1 base sa φ·ψ=-1 at k=42 even)\n\n";

    // ============================================
    // ANG TOTOONG SEPARATOR
    // ============================================
    std::cout << "TOTOONG SEPARATOR:\n";
    std::cout << "===================\n\n";

    // Ang φ^k at ψ^k ay may relasyon: φ^k · ψ^k = 1
    // Kaya: ψ^k = 1/φ^k (mod Q)
    //
    // Para sa x = a·φ^k + b·ψ^k:
    // x · φ^(-k) = a + b·ψ^k·φ^(-k) = a + b·1 = a + b
    // x · ψ^(-k) = a·φ^k·ψ^(-k) + b = a + b
    //
    // PERO: Kung gagamitin natin ang DALAWANG equations:
    // eq1: x · φ^(-k) = a + b
    // eq2: x · ψ^(-k) = a + b
    //
    // Pareho — walang bagong impormasyon.
    //
    // ANG SUSI: kailangan ng ASYMMETRIC na basis
    // Hindi φ^k at ψ^k, kundi φ^k at φ^(-k)

    std::cout << "Asymmetric basis (φ^k, φ^(-k)):\n\n";

    // Kung x = a·φ^k + b·φ^(-k):
    // x · φ^(-k) = a + b·φ^(-2k)
    // x · φ^k = a·φ^(2k) + b
    //
    // May 2 equations at 2 unknowns → solvable!

    NTL::ZZ inv_phi_2k = NTL::InvMod((phi_k * phi_k) % Q, Q);
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;

    // Test: x = φ^k (a=1, b=0)
    NTL::ZZ x = phi_k;
    NTL::ZZ eq1 = (x * inv_phi_k) % Q;  // = a + b·φ^(-2k) = 1 + 0 = 1
    NTL::ZZ eq2 = (x * phi_k) % Q;       // = a·φ^(2k) + b = φ^(2k) + 0

    std::cout << "x = φ^k: eq1 = " << eq1 << " (expected 1)\n";
    std::cout << "          eq2 = " << eq2 << " (expected φ^(2k) = " << phi_2k << ")\n\n";

    // ============================================
    // NOISE SEPARATION WITH ASYMMETRIC BASIS
    // ============================================
    std::cout << "NOISE SEPARATION (Asymmetric):\n";
    std::cout << "===============================\n\n";

    // Ang signal ay nasa φ^k direction
    // Ang noise ay nasa φ^(-k) direction  
    // ct = m·φ^k + e·φ^(-k)
    //
    // Decode: ct · φ^(-k) = m + e·φ^(-2k)
    // Ang e·φ^(-2k) ay maliit kung e ay maliit at φ^(-2k) ay maliit

    for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
        NTL::ZZ ct = (NTL::to_ZZ(1) * phi_k + e * inv_phi_k) % Q;
        NTL::ZZ decoded = (ct * inv_phi_k) % Q;
        
        // Ang decoded ay m + e·φ^(-2k)
        // Para m=1: decoded ≈ 1 (dahil e·φ^(-2k) ay maliit)
        std::cout << "  e=" << e << ": decoded=" << decoded << "\n";
    }

    return 0;
}
