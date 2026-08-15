#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "PROJECTION OPERATORS DEBUG\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi_zz = (NTL::to_ZZ(1) - phi_zz + Q) % Q;
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "ψ = " << psi_zz << "\n\n";
    
    // ========== VERIFY φ at ψ ==========
    std::cout << "VERIFY φ² = φ+1:\n";
    NTL::ZZ phi_sq = (phi_zz * phi_zz) % Q;
    std::cout << "  φ² = " << phi_sq << "\n";
    std::cout << "  φ+1 = " << (phi_zz + 1) % Q << "\n";
    std::cout << "  Match: " << (phi_sq == (phi_zz + 1) % Q ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::cout << "VERIFY ψ² = ψ+1:\n";
    NTL::ZZ psi_sq = (psi_zz * psi_zz) % Q;
    std::cout << "  ψ² = " << psi_sq << "\n";
    std::cout << "  ψ+1 = " << (psi_zz + 1) % Q << "\n";
    std::cout << "  Match: " << (psi_sq == (psi_zz + 1) % Q ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::cout << "VERIFY φ·ψ = -1:\n";
    NTL::ZZ phi_psi = (phi_zz * psi_zz) % Q;
    std::cout << "  φ·ψ = " << phi_psi << "\n";
    std::cout << "  Q-1 = " << Q-1 << "\n";
    std::cout << "  Match: " << (phi_psi == Q-1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::cout << "VERIFY φ + ψ = 1:\n";
    NTL::ZZ phi_plus_psi = (phi_zz + psi_zz) % Q;
    std::cout << "  φ+ψ = " << phi_plus_psi << "\n";
    std::cout << "  Match: " << (phi_plus_psi == 1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== CORRECT PROJECTION OPERATORS ==========
    // Ang tamang projection para sa direct sum Z_Q[φ] = Z_Q·e₁ ⊕ Z_Q·e₂
    // e₁ = (φ-ψ)⁻¹·(φ·ψ - ψ·φ) ... actually:
    // e₁ = (φ-ψ)⁻¹ · (-ψ)  — projects sa φ direction
    // e₂ = (φ-ψ)⁻¹ · φ    — projects sa ψ direction
    
    NTL::ZZ diff = (phi_zz - psi_zz + Q) % Q;
    NTL::ZZ inv_diff = NTL::InvMod(diff, Q);
    
    // Correct e₁: e₁·φ = 1, e₁·ψ = 0
    NTL::ZZ e1_correct = ((-psi_zz + Q) % Q * inv_diff) % Q;
    
    // Correct e₂: e₂·φ = 0, e₂·ψ = 1
    NTL::ZZ e2_correct = (phi_zz * inv_diff) % Q;
    
    std::cout << "CORRECT PROJECTION OPERATORS:\n";
    std::cout << "  e₁ = " << e1_correct << "\n";
    std::cout << "  e₂ = " << e2_correct << "\n\n";
    
    // Verify e₁·φ = 1
    NTL::ZZ e1_phi = (e1_correct * phi_zz) % Q;
    std::cout << "VERIFY e₁·φ = 1:\n";
    std::cout << "  e₁·φ = " << e1_phi << "\n";
    std::cout << "  Match: " << (e1_phi == 1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify e₁·ψ = 0
    NTL::ZZ e1_psi = (e1_correct * psi_zz) % Q;
    std::cout << "VERIFY e₁·ψ = 0:\n";
    std::cout << "  e₁·ψ = " << e1_psi << "\n";
    std::cout << "  Match: " << (e1_psi == 0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify e₂·φ = 0
    NTL::ZZ e2_phi = (e2_correct * phi_zz) % Q;
    std::cout << "VERIFY e₂·φ = 0:\n";
    std::cout << "  e₂·φ = " << e2_phi << "\n";
    std::cout << "  Match: " << (e2_phi == 0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify e₂·ψ = 1
    NTL::ZZ e2_psi = (e2_correct * psi_zz) % Q;
    std::cout << "VERIFY e₂·ψ = 1:\n";
    std::cout << "  e₂·ψ = " << e2_psi << "\n";
    std::cout << "  Match: " << (e2_psi == 1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify e₁ + e₂ = 1
    NTL::ZZ e1_plus_e2 = (e1_correct + e2_correct) % Q;
    std::cout << "VERIFY e₁ + e₂ = 1:\n";
    std::cout << "  e₁+e₂ = " << e1_plus_e2 << "\n";
    std::cout << "  Match: " << (e1_plus_e2 == 1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify e₁·e₂ = 0
    NTL::ZZ e1_e2 = (e1_correct * e2_correct) % Q;
    std::cout << "VERIFY e₁·e₂ = 0:\n";
    std::cout << "  e₁·e₂ = " << e1_e2 << "\n";
    std::cout << "  Match: " << (e1_e2 == 0 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== TEST TRACE-ZERO NOISE ==========
    std::cout << "TEST TRACE-ZERO NOISE:\n";
    NTL::ZZ noise = (phi_zz - psi_zz + Q) % Q;  // φ-ψ
    
    NTL::ZZ e1_noise = (e1_correct * noise) % Q;
    std::cout << "  e₁·(φ-ψ) = " << e1_noise << " (should be 1-0 = 1)\n\n";
    
    // Hmm, so φ-ψ ay HINDI trace-zero!
    // Ang trace-zero ay a·φ + b·ψ kung saan a+b=0
    // So noise = φ - ψ = 1·φ + (-1)·ψ, trace = 1 + (-1) = 0 ✓
    // Pero e₁·(φ-ψ) = e₁·φ - e₁·ψ = 1 - 0 = 1, HINDI 0!
    
    // Para maging perpendicular sa message (φ direction):
    // Kailangan: e₁·noise = 0
    // So noise = r·ψ (pure ψ direction)
    
    std::cout << "TAMANG NOISE: r·ψ (pure ψ direction)\n";
    NTL::ZZ noise2 = psi_zz;  // ψ
    NTL::ZZ e1_noise2 = (e1_correct * noise2) % Q;
    std::cout << "  e₁·ψ = " << e1_noise2 << " (should be 0)\n";
    
    return 0;
}
