#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "GOLDEN RATIO AUTO-CORRECTNESS TEST\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("4294967291"); // Valid 32-bit prime ≡ 1 mod 5
    
    // Compute φ at ψ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    std::cout << "φ = " << phi << "\n";
    std::cout << "ψ = " << psi << "\n\n";
    
    // Verify φ properties
    std::cout << "φ² mod Q = " << (phi*phi) % Q << " (should be " << (phi+1)%Q << ")\n";
    std::cout << "φ·ψ mod Q = " << (phi*psi) % Q << " (should be " << Q-1 << ")\n";
    std::cout << "φ+ψ mod Q = " << (phi+psi) % Q << " (should be 1)\n\n";
    
    // Auto-correctness test
    // Kung v = φ, dapat NAND(v,v) = 0
    NTL::ZZ golden_plain = phi;
    NTL::ZZ inv_golden = NTL::InvMod(golden_plain, Q);
    
    // NAND(1,1) = golden_plain - (golden_plain * golden_plain) * inv_golden
    NTL::ZZ nand_result = (golden_plain - (golden_plain * golden_plain) % Q * inv_golden) % Q;
    if (nand_result < 0) nand_result += Q;
    
    std::cout << "NAND(1,1) = " << nand_result << "\n";
    std::cout << "Expected: 0\n";
    
    // Auto-correctness: kung ang result ay nasa maling orbit, i-correct
    // Kung result = golden_plain (dapat 0), ang error ay φ-1 = ψ
    // I-multiply sa ψ para i-normalize
    NTL::ZZ corrected = (nand_result * psi) % Q;
    if (corrected < 0) corrected += Q;
    
    std::cout << "\nCorrected: " << corrected << "\n";
    std::cout << "Dist to 0: " << ((corrected < Q/2) ? corrected : Q-corrected) << "\n";
    std::cout << "Dist to φ: " << ((corrected > golden_plain) ? corrected-golden_plain : golden_plain-corrected) << "\n";
    
    return 0;
}
