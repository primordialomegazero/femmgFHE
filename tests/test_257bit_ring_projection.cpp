#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "RING ELEMENT PROJECTION OPERATORS\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi_zz = (NTL::to_ZZ(1) - phi_zz + Q) % Q;
    
    std::cout << "φ = " << phi_zz << "\n";
    std::cout << "ψ = " << psi_zz << "\n\n";
    
    // ========== TAMANG PROJECTION: RING ELEMENTS ==========
    // Kailangan natin ng e₁ = a₁ + b₁φ at e₂ = a₂ + b₂φ
    // na may properties:
    // e₁·φ = φ (identity sa φ direction)
    // e₁·ψ = 0 (zero sa ψ direction)
    // e₂·φ = 0
    // e₂·ψ = ψ
    
    // Solve for e₁ = a₁ + b₁φ:
    // (a₁ + b₁φ)·φ = φ
    // a₁φ + b₁φ² = φ
    // a₁φ + b₁(φ+1) = φ
    // (a₁ + b₁)φ + b₁ = φ
    // So: a₁ + b₁ = 1 at b₁ = 0 → a₁ = 1, b₁ = 0
    
    // Actually, sa ring Z_Q[φ]/(φ²-φ-1):
    // Ang identity element ay 1
    // Ang projection sa φ direction ay dapat: e₁·(m·φ + n·ψ) = m·φ
    
    // Sa basis {1, φ}: e₁ = c + d·φ
    // e₁·φ = (c + dφ)φ = cφ + dφ² = cφ + d(φ+1) = d + (c+d)φ
    // Dapat: e₁·φ = φ → d=0, c+d=1 → c=1, d=0
    // So e₁ = 1???
    
    // Hindi ito tama. Ang issue ay hindi tayo sa CRT decomposition
    // ng scalar ring, kundi sa ring mismo.
    
    // Sa ring Z_Q[φ], ang φ at ψ ay hindi na scalars
    // Sila ay elements na may relation φ²=φ+1, ψ²=ψ+1, φ+ψ=1, φ·ψ=-1
    
    // Ang tamang idempotent elements ay:
    // e₁ = (1 + (φ-ψ)/√5) / 2  (projection sa φ)
    // e₂ = (1 - (φ-ψ)/√5) / 2  (projection sa ψ)
    
    // Kailangan natin ng √5 sa ring
    NTL::ZZ sqrt5_zz = sqrt5;
    
    // e₁ = (1 + (φ-ψ)/√5) / 2
    NTL::ZZ phi_minus_psi = (phi_zz - psi_zz + Q) % Q;
    NTL::ZZ inv_sqrt5 = NTL::InvMod(sqrt5_zz, Q);
    NTL::ZZ ratio = (phi_minus_psi * inv_sqrt5) % Q;
    
    NTL::ZZ e1_a = (NTL::to_ZZ(1) + ratio) % Q * inv2 % Q;  // coefficient ng 1
    NTL::ZZ e1_b = NTL::to_ZZ(0);  // coefficient ng φ (since e₁ ay scalar sa basis na ito)
    
    // Actually, e₁ bilang scalar sa Z_Q ay:
    NTL::ZZ e1_scalar = e1_a;
    
    std::cout << "RING PROJECTION e₁ (as scalar): " << e1_scalar << "\n";
    
    // Verify: e₁² = e₁ (idempotent)
    NTL::ZZ e1_sq = (e1_scalar * e1_scalar) % Q;
    std::cout << "e₁² = e₁: " << (e1_sq == e1_scalar ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ========== SIMPLER APPROACH: DIRECT BASIS ==========
    std::cout << "DIRECT BASIS APPROACH:\n";
    std::cout << "Sa ring Z_Q[φ], ang element ay a + bφ\n";
    std::cout << "Multiplication: (a+bφ)(c+dφ) = ac+(ad+bc+bd)φ + bd·1\n";
    std::cout << "= (ac+bd) + (ad+bc+bd)φ\n\n";
    
    // Ang message ay naka-encode sa φ coefficient
    // Encrypt(1) = φ = (0, 1) in (a,b) basis
    // Encrypt(0) = 0 = (0, 0)
    
    // Noise: r·ψ = r·(1-φ) = (r, -r) in (a,b) basis
    // This is perpendicular sa message direction!
    
    std::cout << "EMERGENT PROPERTY: ψ = (1, -1) in (a,b) basis\n";
    std::cout << "φ = (0, 1) in (a,b) basis\n";
    std::cout << "ψ = (1, -1) in (a,b) basis\n";
    std::cout << "Dot product φ·ψ = 0·1 + 1·(-1) = -1 (NOT orthogonal!)\n\n";
    
    // Pero sa TRACE sense:
    // Tr(φ·ψ) = Tr(-1) = -2 ≠ 0
    // Sa NORM sense:
    // Norm(φ) = φ·ψ = -1
    // Norm(ψ) = φ·ψ = -1
    
    std::cout << "KEY INSIGHT: Noise sa ψ direction ay may Norm = Norm(ψ) = -1\n";
    std::cout << "Message sa φ direction ay may Norm = Norm(φ) = -1\n";
    std::cout << "Sila ay CONJUGATE, hindi orthogonal!\n\n";
    
    // ========== SIMPLE FHE NA GUMAGANA ==========
    std::cout << "SIMPLE FHE (conjugate-based):\n";
    std::cout << "Encrypt(m) = m·φ\n";
    std::cout << "Decrypt(ct) = ct·ψ mod Q, check if close to -m\n";
    std::cout << "Kasi φ·ψ = -1, so (m·φ)·ψ = -m\n\n";
    
    // Test
    NTL::ZZ ct1 = phi_zz;  // Encrypt(1) = φ
    NTL::ZZ dec1 = (ct1 * psi_zz) % Q;  // φ·ψ = -1 ≡ Q-1
    
    std::cout << "Encrypt(1) = φ = " << ct1 << "\n";
    std::cout << "Decrypt: φ·ψ = " << dec1 << "\n";
    std::cout << "Expected: Q-1 = " << Q-1 << "\n";
    std::cout << "Match: " << (dec1 == Q-1 ? "YES ✓" : "NO ✗") << "\n";
    
    return 0;
}
