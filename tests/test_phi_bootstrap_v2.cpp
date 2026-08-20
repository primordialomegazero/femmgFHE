// φ-NATURAL BOOTSTRAPPING V2
// 1. Decode: d = ct · φ^(-k) = m + e
// 2. Noise Extract: n = ct mod φ^k = e
// 3. Clean: m = d - n
//
// COST: 1 multiplication + 1 modular reduction

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-NATURAL BOOTSTRAP V2\n";
    std::cout << "  Decode - Noise Extract - Clean\n";
    std::cout << "========================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("1152921504606847009");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 10; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);

    std::cout << "φ^10 = " << phi_k << "\n";
    std::cout << "ψ^10 = " << psi_k << "\n";
    std::cout << "φ^(-10) = " << inv_phi_k << "\n\n";

    // ============================================
    // φ-BOOTSTRAPPING TEST
    // ============================================
    std::cout << "φ-BOOTSTRAPPING TEST:\n";
    std::cout << "=====================\n\n";

    for (NTL::ZZ e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(10), NTL::to_ZZ(100), NTL::to_ZZ(1000000)}) {
        // ct = m·φ^k + e·ψ^k, m=1
        NTL::ZZ ct = (phi_k + e * psi_k) % Q;

        // STEP 1: Decode — d = ct · φ^(-k)
        NTL::ZZ d = (ct * inv_phi_k) % Q;

        // STEP 2: Noise Extract — n = ct mod φ^k
        NTL::ZZ n = ct % phi_k;

        // STEP 3: Clean — m = d - n (mod Q)
        NTL::ZZ m = (d - n + Q) % Q;

        std::cout << "  e=" << e << ":\n";
        std::cout << "    ct = " << ct << "\n";
        std::cout << "    decoded (m+e) = " << d << "\n";
        std::cout << "    noise (e) = " << n << "\n";
        std::cout << "    cleaned m = " << m << "\n";
        std::cout << "    Expected m = 1\n";
        std::cout << "    Match: " << (m == 1 ? "YES ✓" : "NO ✗") << "\n\n";
    }

    return 0;
}
