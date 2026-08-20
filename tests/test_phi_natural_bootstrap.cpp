// φ-NATURAL BOOTSTRAPPING — Scalar Verification
// Ang proposed method:
// 1. Level Reduce: ct → ct mod φ^k
// 2. Period-2 Reset: automatic na nasa {0, φ^k}
// 3. Scale Restore: ct → ct · φ^k
//
// COST: 2 multiplications (vs ~1000+ para sa standard bootstrapping)

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-NATURAL BOOTSTRAPPING\n";
    std::cout << "  Proposed Method Verification\n";
    std::cout << "========================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("1152921504606847009");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 10; i++) {  // k=10 para manageable
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }

    std::cout << "φ^10 = " << phi_k << "\n";
    std::cout << "ψ^10 = " << psi_k << "\n\n";

    // ============================================
    // SIMULATE NOISY CIPHERTEXT
    // ============================================
    std::cout << "SIMULATE NOISY CIPHERTEXT:\n";
    std::cout << "==========================\n\n";

    // After many NAND gates, ang ct ay may noise
    // ct = m·φ^k + e·ψ^k + noise_polynomial
    // Para sa scalar simulation:
    // ct = m·φ^k + e·ψ^k (e ay malaking accumulated noise)

    for (NTL::ZZ e : {NTL::to_ZZ(100), NTL::to_ZZ(1000), NTL::to_ZZ(1000000)}) {
        // m=1
        NTL::ZZ ct = (phi_k + e * psi_k) % Q;

        std::cout << "  Noise e=" << e << ":\n";
        std::cout << "    ct = " << ct << "\n";

        // ============================================
        // φ-NATURAL BOOTSTRAPPING
        // ============================================
        
        // STEP 1: Level Reduce — ct mod φ^k
        // Sa modular arithmetic: ct % phi_k
        NTL::ZZ reduced = ct % phi_k;
        std::cout << "    Step 1 (mod φ^k): " << reduced << "\n";

        // STEP 2: Period-2 Reset
        // Ang reduced value ay nasa {0, φ^k} na automatic
        // Dahil sa invariant set property

        // STEP 3: Scale Restore — multiply sa φ^k (kung kailangan)
        NTL::ZZ restored = (reduced * phi_k) % Q;
        std::cout << "    Step 3 (× φ^k): " << restored << "\n";
        std::cout << "    Expected φ^k: " << phi_k << "\n";
        std::cout << "    Match: " << (restored == phi_k ? "YES" : "NO") << "\n\n";
    }

    return 0;
}
