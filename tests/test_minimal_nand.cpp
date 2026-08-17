// MINIMAL NAND TEST — direct φ encoding, walang RLWE noise
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    // Direct φ encoding
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    NTL::ZZ_p phi_p = NTL::to_ZZ_p(phi);
    NTL::ZZ_p inv_phi = NTL::to_ZZ_p(NTL::InvMod(phi, Q));
    
    std::cout << "φ = " << phi << "\n";
    std::cout << "ψ = " << psi << "\n";
    std::cout << "φ·ψ mod Q = " << (phi * psi) % Q << " (should be Q-1)\n\n";
    
    // MINIMAL NAND: NAND(1,1) = φ - φ²·φ⁻¹ = φ - φ = 0
    NTL::ZZ_p phi_sq = phi_p * phi_p;
    NTL::ZZ_p nand_11 = phi_p - phi_sq * inv_phi;
    
    std::cout << "NAND(1,1) = φ - φ²·φ⁻¹ = " << NTL::rep(nand_11) << " (should be 0)\n";
    
    // MINIMAL NAND: NAND(0,0) = φ - 0·0·φ⁻¹ = φ
    NTL::ZZ_p nand_00 = phi_p - NTL::to_ZZ_p(0) * NTL::to_ZZ_p(0) * inv_phi;
    std::cout << "NAND(0,0) = φ - 0 = " << NTL::rep(nand_00) << " (should be φ)\n";
    
    // MINIMAL NOT: NOT(1) = NAND(1,1) = 0
    std::cout << "NOT(1) = " << NTL::rep(nand_11) << " (should be 0)\n";
    
    // MINIMAL NOT: NOT(0) = NAND(0,0) = φ
    std::cout << "NOT(0) = " << NTL::rep(nand_00) << " (should be φ)\n\n";
    
    // DEEP CHAIN: NOT(NOT(NOT(1))) = ?
    NTL::ZZ_p current = phi_p;  // Start sa 1
    std::cout << "Deep chain:\n";
    for (int i = 0; i <= 10; i++) {
        // NAND(x,x) = φ - x²·φ⁻¹
        NTL::ZZ_p x_sq = current * current;
        current = phi_p - x_sq * inv_phi;
        std::cout << "  Depth " << i << ": " << NTL::rep(current) << "\n";
    }
    
    return 0;
}
