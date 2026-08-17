// NON-INTERACTIVE CHECK: Pwede bang NAND direkta sa outer?
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "NON-INTERACTIVE CHECK\n";
    std::cout << "====================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    
    std::cout << "1. ANG SETUP:\n";
    std::cout << "   outer = inner + e·ψ^k + r·Q\n";
    std::cout << "   kung saan inner ∈ {0, φ^k}\n\n";
    
    std::cout << "2. SUBUKAN: NAND sa outer nang walang decrypt\n";
    std::cout << "   NAND(outer_a, outer_b) = φ^k - outer_a·outer_b·φ^(-k)\n";
    std::cout << "   = φ^k - (inner_a + noise_a)(inner_b + noise_b)·φ^(-k)\n";
    std::cout << "   = φ^k - inner_a·inner_b·φ^(-k) - cross_terms - noise_terms\n";
    std::cout << "   = inner_NAND - cross_terms - noise_terms\n\n";
    
    std::cout << "3. ANG CROSS TERMS:\n";
    std::cout << "   cross_terms = (inner_a·noise_b + noise_a·inner_b)·φ^(-k)\n";
    std::cout << "   noise_terms = noise_a·noise_b·φ^(-k)\n\n";
    
    std::cout << "4. ANG PROBLEM:\n";
    std::cout << "   Ang cross_terms ay depende sa inner values.\n";
    std::cout << "   Kung inner_a = 0, inner_b = φ^k:\n";
    std::cout << "     cross = (0·noise_b + noise_a·φ^k)·φ^(-k) = noise_a\n";
    std::cout << "   Kung inner_a = φ^k, inner_b = φ^k:\n";
    std::cout << "     cross = (φ^k·noise_b + noise_a·φ^k)·φ^(-k) = noise_a + noise_b\n";
    std::cout << "   → MAGKAIBA ang cross terms sa magkaibang input!\n";
    std::cout << "   → Hindi ma-remove nang walang decrypt\n\n";
    
    std::cout << "5. RESULT:\n";
    std::cout << "   Ang NAND ay Nangangailangan ng decrypt ng outer\n";
    std::cout << "   para malaman ang inner values.\n";
    std::cout << "   → HINDI pa non-interactive\n";
    std::cout << "   → Kailangan ng BOOTSTRAPPING sa outer layer\n\n";
    
    std::cout << "6. ANO ANG KAILANGAN:\n";
    std::cout << "   Para sa non-interactive FHE:\n";
    std::cout << "   - Bootstrapping para i-refresh ang outer\n";
    std::cout << "   - O ibang encoding na may natural na NAND\n";
    std::cout << "   - O homomorphic decryption ng outer sa inner\n\n";
    
    std::cout << "=== HINDI PA NON-INTERACTIVE ===\n";
    
    return 0;
}
