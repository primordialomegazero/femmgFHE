// φ-NOISE PROJECTION — Ciphertext Level
// Hanapin kung may natural na noise reduction sa φ-structure
//
// ANG KEY IDEA:
// Ang ciphertext ay polynomial: ct(x) = m + e(x)
// Ang φ at ψ ay roots ng x² - x - 1 = 0
// Kung i-evaluate natin ang ct sa φ at ψ:
//   ct(φ) = m + e(φ)
//   ct(ψ) = m + e(ψ)
//
// Ang difference:
//   ct(φ) - ct(ψ) = e(φ) - e(ψ)
//   ct(φ) + ct(ψ) = 2m + e(φ) + e(ψ)
//
// Kung ang noise ay symmetric sa φ at ψ:
//   e(φ) = e(ψ)
//   → ct(φ) - ct(ψ) = 0 (noise cancels!)
//   → ct(φ) + ct(ψ) = 2m (signal doubles!)

#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-NOISE PROJECTION (Ciphertext Level)\n";
    std::cout << "========================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("1152921504606847009");
    NTL::ZZ_p::init(Q);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    std::cout << "φ = " << phi << "\n";
    std::cout << "ψ = " << psi << "\n\n";

    // ============================================
    // POLYNOMIAL EVALUATION
    // ============================================
    // I-verify na ang φ at ψ ay roots ng x²-x-1
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi);
    NTL::ZZ_p psi_p = NTL::to_ZZ_p(psi);

    NTL::ZZ_p phi_sq = phi_p * phi_p;
    NTL::ZZ_p phi_eq = phi_sq - phi_p - NTL::to_ZZ_p(1);
    std::cout << "φ² - φ - 1 = " << phi_eq << " (dapat 0)\n";

    NTL::ZZ_p psi_sq = psi_p * psi_p;
    NTL::ZZ_p psi_eq = psi_sq - psi_p - NTL::to_ZZ_p(1);
    std::cout << "ψ² - ψ - 1 = " << psi_eq << " (dapat 0)\n\n";

    // ============================================
    // NOISE PROJECTION TEST
    // ============================================
    std::cout << "NOISE PROJECTION TEST:\n";
    std::cout << "======================\n\n";

    // Simulate ciphertext: ct(x) = m + e·x + e·x² + ... (noise polynomial)
    // Para sa simple test: ct(x) = m + e·x
    // ct(φ) = m + e·φ
    // ct(ψ) = m + e·ψ

    for (int m : {0, 1}) {
        for (NTL::ZZ e : {NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10)}) {
            // ct(φ) = m + e·φ
            NTL::ZZ_p ct_phi_val = NTL::to_ZZ_p(m) + NTL::to_ZZ_p(e) * phi_p;
            // ct(ψ) = m + e·ψ
            NTL::ZZ_p ct_psi_val = NTL::to_ZZ_p(m) + NTL::to_ZZ_p(e) * psi_p;

            // Difference: e(φ) - e(ψ) = e·φ - e·ψ = e·(φ-ψ) = e·√5
            NTL::ZZ_p diff = ct_phi_val - ct_psi_val;
            // Sum: 2m + e(φ) + e(ψ) = 2m + e·(φ+ψ) = 2m + e·1 = 2m + e
            NTL::ZZ_p sum = ct_phi_val + ct_psi_val;

            std::cout << "  m=" << m << ", e=" << e << ":\n";
            std::cout << "    ct(φ) = " << ct_phi_val << "\n";
            std::cout << "    ct(ψ) = " << ct_psi_val << "\n";
            std::cout << "    diff = " << diff << " (e·√5 = " << (NTL::to_ZZ_p(e) * NTL::to_ZZ_p(sqrt5)) << ")\n";
            std::cout << "    sum = " << sum << " (2m+e = " << (2*m + NTL::to_long(e)) << ")\n\n";
        }
    }

    // ============================================
    // ANG KEY: MAY SEPARATION BA?
    // ============================================
    std::cout << "KEY INSIGHT:\n";
    std::cout << "=============\n\n";
    std::cout << "  diff = e·√5 (puro noise — walang signal!)\n";
    std::cout << "  sum = 2m + e (signal + noise)\n\n";
    std::cout << "  Kung makuha natin ang e mula sa diff:\n";
    std::cout << "  e = diff / √5\n";
    std::cout << "  At i-subtract sa sum:\n";
    std::cout << "  sum - diff/√5 = 2m + e - e = 2m\n";
    std::cout << "  m = (sum - diff/√5) / 2\n\n";

    // Verify
    NTL::ZZ_p inv_sqrt5 = NTL::inv(NTL::to_ZZ_p(sqrt5));
    NTL::ZZ_p inv_2 = NTL::inv(NTL::to_ZZ_p(2));

    std::cout << "RECOVERY TEST:\n";
    std::cout << "===============\n\n";

    for (int m : {0, 1}) {
        for (NTL::ZZ e : {NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10)}) {
            NTL::ZZ_p ct_phi_val = NTL::to_ZZ_p(m) + NTL::to_ZZ_p(e) * phi_p;
            NTL::ZZ_p ct_psi_val = NTL::to_ZZ_p(m) + NTL::to_ZZ_p(e) * psi_p;
            
            NTL::ZZ_p diff = ct_phi_val - ct_psi_val;
            NTL::ZZ_p sum = ct_phi_val + ct_psi_val;
            
            NTL::ZZ_p e_recovered = diff * inv_sqrt5;
            NTL::ZZ_p m_recovered = (sum - e_recovered) * inv_2;
            
            std::cout << "  m=" << m << ", e=" << e 
                      << " → recovered m=" << m_recovered
                      << ", recovered e=" << e_recovered << "\n";
        }
    }

    return 0;
}
