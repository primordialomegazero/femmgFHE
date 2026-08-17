// φ-ψ NAND DEBUG — I-verify ang NAND formula sa BFV plaintext modulus

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "φ-ψ NAND DEBUG\n";
    std::cout << "==============\n\n";

    NTL::ZZ plaintext_mod = NTL::to_ZZ(65537);

    // Compute φ at ψ sa original Q
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    // I-mod sa plaintext modulus
    NTL::ZZ phi_mod = phi % plaintext_mod;
    NTL::ZZ psi_mod = psi % plaintext_mod;

    std::cout << "1. VALUES:\n";
    std::cout << "   φ mod 65537 = " << phi_mod << "\n";
    std::cout << "   ψ mod 65537 = " << psi_mod << "\n\n";

    // Verify: φ² = φ + 1 sa plaintext modulus
    NTL::ZZ phi_sq = (phi_mod * phi_mod) % plaintext_mod;
    NTL::ZZ phi_plus_1 = (phi_mod + 1) % plaintext_mod;
    std::cout << "2. VERIFY φ² = φ+1 mod 65537:\n";
    std::cout << "   φ² = " << phi_sq << "\n";
    std::cout << "   φ+1 = " << phi_plus_1 << "\n";
    std::cout << "   Match: " << (phi_sq == phi_plus_1 ? "YES ✓" : "NO ✗") << "\n\n";

    // NAND(φ,φ) = 1 - φ² = 1 - (φ+1) = -φ = ψ
    NTL::ZZ nand_phi = (NTL::to_ZZ(1) - phi_sq) % plaintext_mod;
    if (nand_phi < 0) nand_phi += plaintext_mod;
    std::cout << "3. NAND(φ,φ):\n";
    std::cout << "   1 - φ² mod 65537 = " << nand_phi << "\n";
    std::cout << "   ψ = " << psi_mod << "\n";
    std::cout << "   Match: " << (nand_phi == psi_mod ? "YES ✓" : "NO ✗") << "\n\n";

    // Verify: ψ ay negatibong φ?
    NTL::ZZ neg_phi = (plaintext_mod - phi_mod) % plaintext_mod;
    std::cout << "4. ψ ba ay -φ?\n";
    std::cout << "   -φ mod 65537 = " << neg_phi << "\n";
    std::cout << "   ψ = " << psi_mod << "\n";
    std::cout << "   Match: " << (neg_phi == psi_mod ? "YES ✓" : "NO ✗") << "\n\n";

    // Check original φ at ψ relationship
    std::cout << "5. ORIGINAL RELATIONSHIP (sa Q):\n";
    std::cout << "   φ + ψ mod Q = " << ((phi + psi) % Q) << " (dapat 1)\n";
    
    NTL::ZZ phi_plus_psi_mod = (phi_mod + psi_mod) % plaintext_mod;
    std::cout << "   φ + ψ mod 65537 = " << phi_plus_psi_mod << " (dapat 1)\n\n";

    std::cout << "6. ANG PROBLEM:\n";
    std::cout << "   Sa Q: φ + ψ = 1\n";
    std::cout << "   Sa 65537: φ + ψ = " << phi_plus_psi_mod << " ≠ 1\n";
    std::cout << "   → Ang φ at ψ ay HINDI na-conserve sa modulo reduction!\n";
    std::cout << "   → Kailangan ng DIRECT φ at ψ sa 65537, hindi mod reduction!\n";

    return 0;
}
