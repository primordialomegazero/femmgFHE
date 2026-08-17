// EXACT φ NAND FORMULA DEBUG
// Ano ang tamang formula para sa period-2?

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "EXACT φ NAND FORMULA DEBUG\n";
    std::cout << "==========================\n\n";
    
    NTL::ZZ p = NTL::to_ZZ(3604481);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + p) % p;
    
    NTL::ZZ inv_phi = NTL::InvMod(phi, p);
    NTL::ZZ inv_psi = NTL::InvMod(psi, p);
    
    std::cout << "φ = " << phi << "\n";
    std::cout << "ψ = " << psi << "\n";
    std::cout << "φ⁻¹ = " << inv_phi << "\n";
    std::cout << "ψ⁻¹ = " << inv_psi << "\n\n";
    
    // Verify φ·ψ = -1
    NTL::ZZ phi_psi = (phi * psi) % p;
    std::cout << "φ·ψ = " << phi_psi << " (dapat p-1 = " << p-1 << ")\n\n";
    
    // Try DIFFERENT NAND formulas:
    std::cout << "1. NAND(a,b) = 1 - a·b:\n";
    NTL::ZZ nand1_phi = (NTL::to_ZZ(1) - phi * phi) % p;
    if (nand1_phi < 0) nand1_phi += p;
    std::cout << "   NAND(φ,φ) = " << nand1_phi << " (ψ=" << psi << ")\n";
    std::cout << "   Match: " << (nand1_phi == psi ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::cout << "2. NAND(a,b) = φ - a·b·φ⁻¹:\n";
    NTL::ZZ nand2_phi = (phi - phi * phi * inv_phi) % p;
    if (nand2_phi < 0) nand2_phi += p;
    std::cout << "   NAND(φ,φ) = " << nand2_phi << " (ψ=" << psi << ")\n";
    std::cout << "   Match: " << (nand2_phi == psi ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::cout << "3. NAND(a,b) = ψ - a·b·ψ⁻¹:\n";
    NTL::ZZ nand3_phi = (psi - phi * phi * inv_psi) % p;
    if (nand3_phi < 0) nand3_phi += p;
    std::cout << "   NAND(φ,φ) = " << nand3_phi << " (ψ=" << psi << ")\n";
    std::cout << "   Match: " << (nand3_phi == psi ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::cout << "4. NAND(a,b) = -a·b (using φ·ψ=-1):\n";
    NTL::ZZ nand4_phi = (p - phi * phi) % p;
    std::cout << "   NAND(φ,φ) = " << nand4_phi << " (ψ=" << psi << ")\n";
    std::cout << "   Match: " << (nand4_phi == psi ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Verify: φ² = φ+1
    std::cout << "5. VERIFY φ² = φ+1:\n";
    NTL::ZZ phi_sq = (phi * phi) % p;
    NTL::ZZ phi_plus = (phi + 1) % p;
    std::cout << "   φ² = " << phi_sq << "\n";
    std::cout << "   φ+1 = " << phi_plus << "\n";
    std::cout << "   Match: " << (phi_sq == phi_plus ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Find what maps φ → ψ
    std::cout << "6. ANO ANG OPERASYON NA φ → ψ?\n";
    std::cout << "   ψ = 1 - φ = " << (NTL::to_ZZ(1) - phi + p) % p << "\n";
    std::cout << "   ψ = -φ² = " << (p - phi_sq) % p << "\n";
    std::cout << "   ψ = φ⁻¹·(-1) = " << (p - inv_phi) % p << "\n\n";
    
    return 0;
}
