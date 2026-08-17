// DUAL BK APPROACH
// BK_φ para sa φ-direction, BK_ψ para sa ψ-direction

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "DUAL BK APPROACH RESEARCH\n";
    std::cout << "=========================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) { phi_k = (phi_k * phi) % Q; psi_k = (psi_k * psi) % Q; }
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;

    std::mt19937_64 rng(42);

    std::cout << "1. ANG DUAL BK CONCEPT:\n";
    std::cout << "   BK_φ = φ^k (o encrypted version)\n";
    std::cout << "   BK_ψ = ψ^k (o encrypted version)\n\n";

    std::cout << "2. BAKIT KAILANGAN NG DALAWA:\n";
    std::cout << "   - φ-direction para sa MESSAGE\n";
    std::cout << "   - ψ-direction para sa NOISE CANCELLATION\n";
    std::cout << "   - Kailangan pareho para sa tamang decryption\n\n";

    // TEST 1: Simple recovery with direct φ^k (hindi encrypted)
    std::cout << "3. TEST 1: DIRECT RECOVERY (baseline)\n";
    NTL::ZZ inner = phi_k;
    NTL::ZZ e_ct = NTL::to_ZZ(7);
    NTL::ZZ ct = inner + e_ct * psi_k;
    
    NTL::ZZ direct_scaled = (ct * phi_k) % Q;
    NTL::ZZ diff = (direct_scaled > phi_2k) ? direct_scaled - phi_2k : phi_2k - direct_scaled;
    std::cout << "   ct·φ^k - φ^(2k) = " << diff << " (dapat " << e_ct << ")\n\n";

    // TEST 2: With BK_φ = φ^k + e_bk·ψ^k
    std::cout << "4. TEST 2: WITH BK_φ (encrypted φ^k)\n";
    NTL::ZZ e_bk = NTL::to_ZZ(5);
    NTL::ZZ BK_phi = phi_k + e_bk * psi_k;
    
    NTL::ZZ hom_dec = (ct * BK_phi) % Q;
    NTL::ZZ hom_scaled = (hom_dec * phi_k) % Q;
    
    std::cout << "   ct·BK_φ·φ^k = " << hom_scaled << "\n";
    std::cout << "   φ^(3k) = " << (phi_k * phi_k * phi_k) % Q << "\n";
    std::cout << "   Difference: " << (hom_scaled - (phi_k * phi_k * phi_k) % Q + Q) % Q << "\n\n";

    // TEST 3: With Dual BK — BK_φ at BK_ψ
    std::cout << "5. TEST 3: DUAL BK (BK_φ + BK_ψ)\n";
    NTL::ZZ e_bk_psi = NTL::to_ZZ(3);
    NTL::ZZ BK_psi = psi_k + e_bk_psi * phi_k;  // encrypted ψ^k
    
    NTL::ZZ dual_dec = (ct * BK_phi + ct * BK_psi) % Q;
    std::cout << "   ct·(BK_φ + BK_ψ) = " << dual_dec << "\n\n";

    std::cout << "6. KEY INSIGHT:\n";
    std::cout << "   Ang BK_ψ ay nagbibigay ng noise component.\n";
    std::cout << "   Kung ma-subtract natin ang noise component,\n";
    std::cout << "   ang message ay mas malinis.\n";

    return 0;
}
