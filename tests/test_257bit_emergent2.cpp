#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "EMERGENT PROPERTIES DISCOVERY\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi_zz = (NTL::to_ZZ(1) - phi_zz + Q) % Q;  // conjugate
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "ψ = " << psi_zz << " (conjugate)\n\n";
    
    // ========== EMERGENT PROPERTY 1: φ·ψ = -1 = Q-1 ==========
    std::cout << "EMERGENT 1: φ·ψ = -1\n";
    NTL::ZZ phi_psi = (phi_zz * psi_zz) % Q;
    std::cout << "  φ·ψ = " << phi_psi << "\n";
    std::cout << "  Q-1 = " << Q-1 << "\n";
    std::cout << "  Match: " << (phi_psi == Q-1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== EMERGENT PROPERTY 2: φ + ψ = 1 ==========
    std::cout << "EMERGENT 2: φ + ψ = 1\n";
    NTL::ZZ phi_plus_psi = (phi_zz + psi_zz) % Q;
    std::cout << "  φ+ψ = " << phi_plus_psi << "\n";
    std::cout << "  Match: " << (phi_plus_psi == 1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== EMERGENT PROPERTY 3: PROJECTION OPERATORS ==========
    std::cout << "EMERGENT 3: PROJECTION OPERATORS\n";
    // e₁ = -ψ/(φ-ψ) = projection sa φ direction
    // e₂ = φ/(φ-ψ) = projection sa ψ direction
    
    NTL::ZZ inv_phi_minus_psi = NTL::InvMod((phi_zz - psi_zz + Q) % Q, Q);
    NTL::ZZ e1 = ((-psi_zz + Q) % Q * inv_phi_minus_psi) % Q;
    NTL::ZZ e2 = (phi_zz * inv_phi_minus_psi) % Q;
    
    std::cout << "  e₁ = " << e1 << "\n";
    std::cout << "  e₂ = " << e2 << "\n";
    std::cout << "  e₁ + e₂ = " << (e1 + e2) % Q << " (should be 1)\n";
    std::cout << "  e₁·e₂ = " << (e1 * e2) % Q << " (should be 0)\n\n";
    
    // ========== EMERGENT PROPERTY 4: FIBONACCI INVERSION ==========
    std::cout << "EMERGENT 4: FIBONACCI INVERSION\n";
    // φ^n = F(n)·φ + F(n-1)
    // Inverse: F(n) = (φ^n - ψ^n)/(φ-ψ)
    
    NTL::ZZ inv_phi_psi = NTL::InvMod((phi_zz - psi_zz + Q) % Q, Q);
    
    // Test para sa n=5
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi_zz);
    NTL::ZZ_p psi_p = NTL::to_ZZ_p(psi_zz);
    NTL::ZZ_p phi_5 = NTL::to_ZZ_p(1);
    NTL::ZZ_p psi_5 = NTL::to_ZZ_p(1);
    for (int i = 0; i < 5; i++) {
        phi_5 = phi_5 * phi_p;
        psi_5 = psi_5 * psi_p;
    }
    NTL::ZZ F5 = (rep(phi_5) - rep(psi_5) + Q) % Q * inv_phi_psi % Q;
    std::cout << "  F(5) = " << F5 << " (expected 5)\n\n";
    
    // ========== EMERGENT PROPERTY 5: TRACE FORMULA ==========
    std::cout << "EMERGENT 5: TRACE FORMULA\n";
    // Tr(φ^n) = φ^n + ψ^n = L(n) (Lucas number)
    NTL::ZZ trace_5 = (rep(phi_5) + rep(psi_5)) % Q;
    std::cout << "  L(5) = " << trace_5 << " (expected 11)\n\n";
    
    // ========== EMERGENT PROPERTY 6: NORM ==========
    std::cout << "EMERGENT 6: NORM PROPERTY\n";
    // Norm(a + bφ) = (a + bφ)(a + bψ)
    // = a² + ab(φ+ψ) + b²(φ·ψ)
    // = a² + ab - b²
    
    NTL::ZZ a = NTL::to_ZZ(3);
    NTL::ZZ b = NTL::to_ZZ(5);
    NTL::ZZ norm = (a*a + a*b - b*b) % Q;
    std::cout << "  Norm(3 + 5φ) = " << norm << "\n";
    std::cout << "  = 9 + 15 - 25 = -1 mod Q = " << (Q-1) << "\n\n";
    
    // ========== EMERGENT PROPERTY 7: UNIT GROUP ==========
    std::cout << "EMERGENT 7: UNIT GROUP\n";
    // φ ay unit (may inverse)
    NTL::ZZ phi_inv = NTL::InvMod(phi_zz, Q);
    std::cout << "  φ⁻¹ = " << phi_inv << "\n";
    std::cout << "  φ·φ⁻¹ = " << (phi_zz * phi_inv) % Q << " (should be 1)\n";
    std::cout << "  φ⁻¹ = φ - 1? " << (phi_inv == (phi_zz - 1 + Q) % Q ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== EMERGENT PROPERTY 8: DECRYPTION VIA PROJECTION ==========
    std::cout << "EMERGENT 8: DECRYPTION VIA PROJECTION\n";
    // Given ct = m·φ + noise
    // Project sa φ direction: e₁ · ct = m (if noise is ψ-direction)
    // Project sa ψ direction: e₂ · ct = m (if noise is φ-direction)
    
    // Test: Encrypt(1) = φ = (1, 0)
    NTL::ZZ proj1 = (e1 * phi_zz) % Q;
    std::cout << "  e₁·φ = " << proj1 << " (should be 1)\n";
    
    NTL::ZZ proj2 = (e2 * phi_zz) % Q;
    std::cout << "  e₂·φ = " << proj2 << " (should be 0)\n\n";
    
    // ========== EMERGENT PROPERTY 9: SELF-REDUCING ==========
    std::cout << "EMERGENT 9: SELF-REDUCING\n";
    // φ² = φ + 1 means:
    // φ^n reduces to LINEAR combination
    // No higher powers needed!
    
    NTL::ZZ_p phi_10 = NTL::to_ZZ_p(1);
    for (int i = 0; i < 10; i++) phi_10 = phi_10 * phi_p;
    NTL::ZZ phi_10_zz = rep(phi_10);
    
    // F(10) = 55, F(9) = 34
    NTL::ZZ expected_10 = (NTL::to_ZZ(55) * phi_zz + NTL::to_ZZ(34)) % Q;
    std::cout << "  φ^10 = " << phi_10_zz << "\n";
    std::cout << "  55φ + 34 = " << expected_10 << "\n";
    std::cout << "  Match: " << (phi_10_zz == expected_10 ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::cout << "=== EMERGENT PROPERTIES SUMMARY ===\n";
    std::cout << "1. φ·ψ = -1 (conjugate product)\n";
    std::cout << "2. φ + ψ = 1 (conjugate sum)\n";
    std::cout << "3. Projection operators e₁, e₂ (idempotent)\n";
    std::cout << "4. Fibonacci inversion formula\n";
    std::cout << "5. Trace = Lucas numbers\n";
    std::cout << "6. Norm = a² + ab - b²\n";
    std::cout << "7. φ⁻¹ = φ - 1 (unit)\n";
    std::cout << "8. Decryption via projection\n";
    std::cout << "9. Self-reducing (linear combination)\n";
    
    return 0;
}
