// φ-CONJUGATE REFRESH — Scalar Verification
// Ang refresh ay 1 multiplication lang
// refresh(ct) = ct · ψ^k
//
// Kung ct = m·φ^k + e·ψ^k:
// refresh(ct) = m·φ^k·ψ^k + e·ψ^(2k)
//             = m·(-1)^k + e·ψ^(2k)
//             = m + e·ψ^(2k)  (k=42 even)
//
// Ang signal ay naging m (hindi na φ^k-scaled)
// Ang noise ay naging e·ψ^(2k)

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-CONJUGATE REFRESH\n";
    std::cout << "  1 Multiplication Refresh\n";
    std::cout << "========================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("1152921504606847009");  // OpenFHE prime
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

    std::cout << "φ^k = " << phi_k << "\n";
    std::cout << "ψ^k = " << psi_k << "\n\n";

    // ============================================
    // CONJUGATE REFRESH TEST
    // ============================================
    std::cout << "CONJUGATE REFRESH TEST:\n";
    std::cout << "=======================\n\n";

    // ct = m·φ^k + e·ψ^k
    // refresh(ct) = ct · ψ^k
    // = m·φ^k·ψ^k + e·ψ^(2k)
    // = m + e·ψ^(2k)

    for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
        // m = 1
        NTL::ZZ ct = (phi_k + e * psi_k) % Q;
        NTL::ZZ refreshed = (ct * psi_k) % Q;

        std::cout << "  m=1, e=" << e << ":\n";
        std::cout << "    ct = " << ct << "\n";
        std::cout << "    refreshed = " << refreshed << "\n";
        std::cout << "    Expected ≈ 1 + e·ψ^(2k) = " 
                  << (NTL::to_ZZ(1) + e * ((psi_k * psi_k) % Q)) % Q << "\n\n";
    }

    // ============================================
    // KEY: Ang refresh ay nagbabago ng SCALE
    // ============================================
    std::cout << "SCALE CHANGE:\n";
    std::cout << "==============\n\n";
    std::cout << "  Bago refresh: signal scale = φ^k = " << phi_k << "\n";
    std::cout << "  Pagkatapos refresh: signal scale = 1\n";
    std::cout << "  Ang susunod na NAND ay dapat nasa scale 1\n\n";

    // Ang refresh ay nagre-encode ng value mula sa φ^k-scale
    // papunta sa 1-scale. Ito ay equivalent sa modulus switching
    // pero gamit ang golden ratio structure!

    return 0;
}
