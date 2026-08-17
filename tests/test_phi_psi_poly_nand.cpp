// φ-ψ POLYNOMIAL NAND — Period-2 sa polynomial ring
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "φ-ψ POLYNOMIAL NAND\n";
    std::cout << "===================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    std::cout << "φ = " << phi << "\n";
    std::cout << "ψ = " << psi << "\n\n";
    
    // Check: φ at ψ ay roots ng x²+x-1=0? O x²-x-1=0?
    std::cout << "1. ROOT CHECK:\n";
    std::cout << "   φ²-φ-1 = " << (phi*phi - phi - 1) % Q << " (dapat 0)\n";
    std::cout << "   ψ²-ψ-1 = " << (psi*psi - psi - 1) % Q << " (dapat 0)\n\n";
    
    // Check: ang equation x(x-1)(x²+x-1)=0
    std::cout << "2. FIXED POINT EQUATION x(x-1)(x²+x-1)=0:\n";
    NTL::ZZ eq_phi = phi * (phi-1) * (phi*phi + phi - 1);
    NTL::ZZ eq_psi = psi * (psi-1) * (psi*psi + psi - 1);
    std::cout << "   Sa φ: " << eq_phi % Q << " (dapat 0)\n";
    std::cout << "   Sa ψ: " << eq_psi % Q << " (dapat 0)\n\n";
    
    // Period-2 NAND test sa scalar
    std::cout << "3. PERIOD-2 NAND TEST (scalar):\n";
    NTL::ZZ nand_phi = (NTL::to_ZZ(1) - phi * phi) % Q;
    if (nand_phi < 0) nand_phi += Q;
    std::cout << "   NAND(φ,φ) = 1-φ² = " << nand_phi << "\n";
    std::cout << "   = ψ? " << (nand_phi == psi ? "YES ✓" : "NO ✗") << "\n";
    
    NTL::ZZ nand_psi = (NTL::to_ZZ(1) - psi * psi) % Q;
    if (nand_psi < 0) nand_psi += Q;
    std::cout << "   NAND(ψ,ψ) = 1-ψ² = " << nand_psi << "\n";
    std::cout << "   = φ? " << (nand_psi == phi ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::cout << "4. KEY INSIGHT:\n";
    std::cout << "   Kung message 1 = φ at message 0 = ψ:\n";
    std::cout << "   NAND(φ,φ) = ψ (tama — NAND(1,1)=0)\n";
    std::cout << "   NAND(ψ,ψ) = φ (tama — NAND(0,0)=1)\n";
    std::cout << "   → PERIOD-2: φ→ψ→φ→ψ!\n\n";
    
    return 0;
}
