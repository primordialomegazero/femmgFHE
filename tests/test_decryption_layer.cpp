// DECRYPTION LAYER
// Homomorphic decryption ng outer → inner
// Evaluator ay may encrypted secret key para sa outer decryption

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "DECRYPTION LAYER TEST\n";
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
    
    std::mt19937_64 rng(42);
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    
    std::cout << "1. ANG DECRYPTION LAYER CONCEPT:\n";
    std::cout << "   Evaluator ay may ENCRYPTED φ^k (hindi plain φ^k)\n";
    std::cout << "   Bootstrapping key: BK = Encrypt(φ^k)\n";
    std::cout << "   Evaluator: ct·BK = inner·φ^k + noise·BK\n";
    std::cout << "   → Homomorphic decryption!\n\n";
    
    std::cout << "2. SIMPLIFIED: Evaluator ay may φ^k mismo\n";
    std::cout << "   (hindi ito secure, pero para sa testing)\n";
    std::cout << "   Evaluator: ct mod Q → then multiply by φ^k\n";
    std::cout << "   → Ito ang ginagawa natin sa current decrypt\n\n";
    
    std::cout << "3. ANG TAMANG SETUP:\n";
    std::cout << "   - Inner encryption: m → m·φ^k (perfect NAND)\n";
    std::cout << "   - Outer encryption: inner → inner + e·ψ^k + r·Q\n";
    std::cout << "   - Bootstrapping key: BK = φ^k (or encrypted version)\n";
    std::cout << "   - Evaluator NAND:\n";
    std::cout << "     a_inner = a_outer mod Q\n";
    std::cout << "     b_inner = b_outer mod Q\n";
    std::cout << "     result = φ^k - (a_inner·b_inner)·φ^(-k)\n";
    std::cout << "     re-encrypt: result + e_new·ψ^k + r_new·Q\n\n";
    
    std::cout << "4. SECURITY ANALYSIS:\n";
    std::cout << "   Kung ang evaluator ay may φ^k:\n";
    std::cout << "   - Makikita niya ang inner values (0 o φ^k)\n";
    std::cout << "   - Makikita niya ang 50/50 pattern!\n";
    std::cout << "   → HINDI SECURE kung φ^k ay public\n\n";
    
    std::cout << "5. ANG TAMANG APPROACH:\n";
    std::cout << "   Evaluator ay may ENCRYPTED φ^k:\n";
    std::cout << "   E(φ^k) = φ^k + e_bk·ψ^k + r_bk·Q\n";
    std::cout << "   Evaluator: ct·E(φ^k) mod Q\n";
    std::cout << "   = inner·φ^k + noise_a·φ^k + inner·e_bk·ψ^k + ...\n";
    std::cout << "   → COMPLEX — cross terms ulit!\n\n";
    
    std::cout << "6. ANG PRACTICAL NA SAGOT:\n";
    std::cout << "   Sa kasalukuyan, ang evaluator ay kailangan ng\n";
    std::cout << "   φ^k para sa NAND. Ito ay SECURITY LEAK.\n";
    std::cout << "   Para sa tunay na FHE, kailangan ng:\n";
    std::cout << "   - Bootstrapping (Gentry)\n";
    std::cout << "   - O KeySwitching (BGV/BFV)\n";
    std::cout << "   - O mas complex na homomorphic decryption\n\n";
    
    std::cout << "=== DECRYPTION LAYER: HINDI PA SOLVED ===\n";
    
    return 0;
}
