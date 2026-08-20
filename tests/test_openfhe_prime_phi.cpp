// TEST OPENFHE PRIME + φ-STRUCTURE
// Ang prime 1152921504606847009 ay may φ-structure
// I-verify kung ang period-2 property ay gumagana dito

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  OPENFHE PRIME + φ-STRUCTURE\n";
    std::cout << "  Period-2 Verification\n";
    std::cout << "========================================\n\n";

    // Ang OpenFHE prime
    NTL::ZZ Q = NTL::to_ZZ("1152921504606847009");

    // Compute √5 at φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    std::cout << "Q = " << Q << "\n";
    std::cout << "√5 = " << sqrt5 << "\n";
    std::cout << "φ = " << phi << "\n";
    std::cout << "ψ = " << psi << "\n\n";

    // Verify φ² = φ + 1
    NTL::ZZ phi_sq = (phi * phi) % Q;
    NTL::ZZ phi_plus_1 = (phi + 1) % Q;
    std::cout << "φ² = " << phi_sq << "\n";
    std::cout << "φ+1 = " << phi_plus_1 << "\n";
    std::cout << "Match: " << (phi_sq == phi_plus_1 ? "YES" : "NO") << "\n\n";

    // Verify φ·ψ = -1
    NTL::ZZ phi_psi = (phi * psi) % Q;
    std::cout << "φ·ψ mod Q = " << phi_psi << "\n";
    std::cout << "Dapat Q-1 = " << Q - 1 << "\n";
    std::cout << "Match: " << (phi_psi == Q - 1 ? "YES" : "NO") << "\n\n";

    // Period-2 test
    std::cout << "PERIOD-2 TEST:\n";
    std::cout << "================\n\n";

    // NAND(x,x) = φ - x²·φ⁻¹
    NTL::ZZ inv_phi = NTL::InvMod(phi, Q);
    auto nand_op = [&](NTL::ZZ x) {
        NTL::ZZ x_sq = (x * x) % Q;
        NTL::ZZ scaled = (x_sq * inv_phi) % Q;
        NTL::ZZ result = (phi - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    // Test: NAND(NAND(φ,φ), NAND(φ,φ)) = φ
    NTL::ZZ not_phi = nand_op(phi);
    NTL::ZZ not_not_phi = nand_op(not_phi);
    std::cout << "NOT(φ) = " << not_phi << "\n";
    std::cout << "NOT(NOT(φ)) = " << not_not_phi << "\n";
    std::cout << "φ = " << phi << "\n";
    std::cout << "Period-2 Match: " << (not_not_phi == phi ? "YES" : "NO") << "\n\n";

    // 100-gate chain
    std::cout << "100-GATE CHAIN (period-2):\n";
    std::cout << "============================\n\n";

    auto current = phi;
    int errors = 0;

    for (int i = 0; i < 100; i++) {
        current = nand_op(current);
        NTL::ZZ expected = (i % 2 == 0) ? NTL::to_ZZ(0) : phi;
        // Actually: after NAND, φ → 0, 0 → φ
        // So expected: i=0 → 0, i=1 → φ, i=2 → 0, ...
        NTL::ZZ exp = (i % 2 == 0) ? NTL::to_ZZ(0) : phi;
        if (current != exp) errors++;
    }

    std::cout << "Errors: " << errors << "/100\n";

    return 0;
}
