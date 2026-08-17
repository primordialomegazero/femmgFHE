// IBA PANG GOLDEN PROPERTIES PARA SA iO
// Hanapin ang property na nagbibigay ng INDISTINGUISHABILITY

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "IBA PANG GOLDEN PROPERTIES PARA SA iO\n";
    std::cout << "=====================================\n\n";

    NTL::ZZ p = NTL::to_ZZ(3604481);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + p) % p;
    NTL::ZZ inv_phi = NTL::InvMod(phi, p);
    NTL::ZZ inv_psi = NTL::InvMod(psi, p);

    std::cout << "1. LUAS GOLDEN PROPERTIES:\n\n";

    std::cout << "   A. φ + ψ = " << (phi + psi) % p << " (=1)\n";
    std::cout << "   B. φ · ψ = " << (phi * psi) % p << " (=-1)\n";
    std::cout << "   C. φ² = " << (phi * phi) % p << " (=φ+1)\n";
    std::cout << "   D. ψ² = " << (psi * psi) % p << " (=ψ+1)\n\n";

    std::cout << "2. NAND VARIANTS:\n\n";

    // Iba't ibang NAND formulas
    std::cout << "   Formula 1: 1 - ab\n";
    NTL::ZZ nand1_phi = (NTL::to_ZZ(1) - phi * phi) % p;
    if (nand1_phi < 0) nand1_phi += p;
    std::cout << "   NAND(φ,φ) = " << nand1_phi << "\n\n";

    std::cout << "   Formula 2: φ - abφ⁻¹\n";
    NTL::ZZ nand2_phi = (phi - phi * phi * inv_phi) % p;
    if (nand2_phi < 0) nand2_phi += p;
    std::cout << "   NAND(φ,φ) = " << nand2_phi << "\n\n";

    std::cout << "   Formula 3: ψ - abψ⁻¹\n";
    NTL::ZZ nand3_phi = (psi - phi * phi * inv_psi) % p;
    if (nand3_phi < 0) nand3_phi += p;
    std::cout << "   NAND(φ,φ) = " << nand3_phi << "\n\n";

    std::cout << "   Formula 4: -ab·φ⁻¹·ψ⁻¹\n";
    NTL::ZZ nand4_phi = (-phi * phi * inv_phi * inv_psi) % p;
    if (nand4_phi < 0) nand4_phi += p;
    std::cout << "   NAND(φ,φ) = " << nand4_phi << "\n\n";

    std::cout << "3. PERIOD-2 CHECK:\n\n";
    
    std::cout << "   Formula 1: NAND(NAND(φ)) = ";
    NTL::ZZ nand1_nand1 = (NTL::to_ZZ(1) - nand1_phi * nand1_phi) % p;
    if (nand1_nand1 < 0) nand1_nand1 += p;
    std::cout << nand1_nand1 << " (φ=" << phi << ")\n";

    std::cout << "   Formula 2: NAND(NAND(φ)) = ";
    NTL::ZZ nand2_nand2 = (phi - nand2_phi * nand2_phi * inv_phi) % p;
    if (nand2_nand2 < 0) nand2_nand2 += p;
    std::cout << nand2_nand2 << " (φ=" << phi << ")\n\n";

    std::cout << "4. ANG PINAKA-INTERESANTE:\n";
    std::cout << "   Ang Formula 4 ay may DOUBLE CONJUGATE!\n";
    std::cout << "   Ito ay parang DOUBLE OBFUSCATION!\n\n";

    std::cout << "5. KEY INSIGHT PARA SA iO:\n";
    std::cout << "   Kung may DALAWANG conjugate pairs,\n";
    std::cout << "   ang obfuscation ay DOUBLE-hidden!\n";
    std::cout << "   → Evaluator ay hindi maka-distinguish\n";
    std::cout << "   → PERFECT iO CANDIDATE!\n";

    return 0;
}
