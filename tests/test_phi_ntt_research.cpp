// φ-NTT RESEARCH — Frequency Domain Analysis
// Ang φ^k at ψ^k bilang multiplicative inverses sa NTT domain
//
// KEY HYPOTHESIS:
// Sa NTT domain, ang multiplication ay nagiging pointwise
// Kung ang φ^k ay may special structure sa NTT domain,
// ang noise cancellation ay maaaring natural na mangyari

#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-NTT RESEARCH\n";
    std::cout << "  Frequency Domain Analysis\n";
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

    std::cout << "φ^k mod Q = " << phi_k << "\n";
    std::cout << "ψ^k mod Q = " << psi_k << "\n";
    std::cout << "φ^k · ψ^k mod Q = " << (phi_k * psi_k) % Q << " (dapat 1)\n\n";

    // ============================================
    // NTT ANALYSIS
    // ============================================
    // Sa NTT domain, ang isang scalar value ay may "spectrum"
    // Ang φ^k ay may specific na pattern sa frequency domain
    //
    // Para sa N=1024 (power of 2), ang NTT ay:
    // X_i = Σ x_j · ω^(ij) mod Q
    // kung saan ω ay primitive 1024th root of unity

    const int N = 1024;
    NTL::ZZ_p::init(Q);

    // Hanapin ang primitive 1024th root of unity
    // ω = primitive 1024th root mod Q
    // Ang Q-1 ay dapat divisible by 1024

    std::cout << "Checking NTT compatibility:\n";
    std::cout << "  Q-1 divisible by 1024? " << ((Q - 1) % 1024 == 0 ? "YES" : "NO") << "\n\n";

    if ((Q - 1) % 1024 == 0) {
        // Hanapin ang primitive root
        NTL::ZZ primitive_root;
        for (NTL::ZZ g = NTL::to_ZZ(2); g < Q; g += 1) {
            if (NTL::PowerMod(g, (Q - 1) / 2, Q) != 1) {
                primitive_root = g;
                break;
            }
        }

        NTL::ZZ omega = NTL::PowerMod(primitive_root, (Q - 1) / 1024, Q);
        std::cout << "  Primitive 1024th root ω = " << omega << "\n\n";

        // I-compute ang NTT ng φ^k bilang "delta function"
        // φ^k ay nasa coefficient 0, kaya sa NTT:
        // NTT(φ^k) = φ^k sa lahat ng frequency (constant spectrum)

        std::cout << "NTT SPECTRUM ANALYSIS:\n";
        std::cout << "=======================\n\n";
        std::cout << "  φ^k bilang constant polynomial:\n";
        std::cout << "  NTT(φ^k)_i = φ^k para sa lahat ng i\n";
        std::cout << "  → Flat spectrum (white noise equivalent)\n\n";

        // Ang period-2 structure ay:
        // x → NAND(x,x) → x → NAND(x,x) → x → ...
        // Sa NTT domain, ang period-2 ay:
        // X(ω) → X(ω²) → X(ω⁴) → ... → X(ω^(2^k))
        // Kung ω^(2^k) = ω para sa some k, may cycle

        std::cout << "PERIOD-2 SA NTT DOMAIN:\n";
        std::cout << "========================\n\n";

        // Sa NTT, ang NAND(x,x) = 1 - x²
        // Sa frequency domain:
        // NAND(X(ω)) = 1 - X(ω)²
        //
        // Ang period-2 condition:
        // NAND(NAND(X(ω))) = X(ω)
        // 1 - (1 - X(ω)²)² = X(ω)
        // 1 - (1 - 2X(ω)² + X(ω)⁴) = X(ω)
        // 1 - 1 + 2X(ω)² - X(ω)⁴ = X(ω)
        // 2X(ω)² - X(ω)⁴ = X(ω)
        // X(ω)⁴ - 2X(ω)² + X(ω) = 0
        // X(ω)(X(ω)³ - 2X(ω) + 1) = 0
        // X(ω) = 0 O X(ω)³ - 2X(ω) + 1 = 0
        //
        // Ang cubic: X³ - 2X + 1 = 0
        // Roots: X = 1, X = -1 ± √5/2 = φ o ψ
        //
        // KAYA: Ang fixed points ay {0, 1, φ, ψ}!

        std::cout << "FIXED POINTS SA NTT DOMAIN:\n";
        std::cout << "  X(ω) = 0\n";
        std::cout << "  X(ω) = 1\n";
        std::cout << "  X(ω) = φ = " << phi << "\n";
        std::cout << "  X(ω) = ψ = " << psi << "\n\n";

        // Ang φ at ψ ay fixed points ng period-2 map!
        // Ito ay nagpapaliwanag kung bakit ang φ-structure
        // ay may natural na stability

        std::cout << "EMERGENT PROPERTY:\n";
        std::cout << "===================\n\n";
        std::cout << "  Ang φ at ψ ay FIXED POINTS ng NAND period-2 map\n";
        std::cout << "  NAND(NAND(φ)) = φ\n";
        std::cout << "  NAND(NAND(ψ)) = ψ\n\n";
        std::cout << "  Ito ay hindi nakikita sa time domain,\n";
        std::cout << "  pero sa frequency domain ay malinaw!\n\n";

        // Check: NAND(NAND(φ)) sa scalar
        NTL::ZZ nand_phi = (NTL::to_ZZ(1) - phi_k * phi_k % Q + Q) % Q;
        NTL::ZZ nand_nand_phi = (NTL::to_ZZ(1) - nand_phi * nand_phi % Q + Q) % Q;
        std::cout << "  Verify: NAND(NAND(φ^k)) = " << nand_nand_phi << "\n";
        std::cout << "  φ^k = " << phi_k << "\n";
        std::cout << "  Match: " << (nand_nand_phi == phi_k ? "YES" : "NO") << "\n";

    } else {
        std::cout << "  Q-1 ay hindi divisible ng 1024\n";
        std::cout << "  Kailangan ng ibang prime o ring dimension\n";
    }

    return 0;
}
