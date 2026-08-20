// ψ-PRIORITY STRATEGY
// ψ ang primary decision, φ ang secondary
// Alamin kung kailan mas tama ang ψ at kailan mas tama ang φ

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  ψ-PRIORITY STRATEGY\n";
    std::cout << "  ψ ang primary, φ ang secondary\n";
    std::cout << "========================================\n\n";

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
    NTL::ZZ inv_psi_k = NTL::InvMod(psi_k, Q);
    NTL::ZZ offset = (phi_k * inv_psi_k) % Q;

    auto encrypt = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };
    auto decode_phi = [&](NTL::ZZ ct) {
        return (ct * inv_phi_k) % Q;
    };
    auto decode_psi = [&](NTL::ZZ ct) {
        return (ct * inv_psi_k) % Q;
    };
    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };
    auto dist = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ d = (a > b) ? a - b : b - a;
        if (d > Q/2) d = Q - d;
        return d;
    };

    // Test sa iba't ibang noise levels
    for (NTL::ZZ e : {NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10)}) {
        std::cout << "INITIAL NOISE e = " << e << "\n";
        std::cout << "=====================\n";

        auto current = encrypt(1, e);
        int psi_correct = 0, phi_correct = 0;
        int total = 0;

        for (int i = 0; i <= 30; i++) {
            NTL::ZZ dec_phi = decode_phi(current);
            NTL::ZZ dec_psi = decode_psi(current);

            int phi_dec = (dist(dec_phi, NTL::to_ZZ(1)) < dist(dec_phi, NTL::to_ZZ(0))) ? 1 : 0;
            int psi_dec = (dist(dec_psi, offset) < dist(dec_psi, NTL::to_ZZ(0))) ? 1 : 0;
            int expected = (i % 2 == 0) ? 1 : 0;

            if (psi_dec == expected) psi_correct++;
            if (phi_dec == expected) phi_correct++;
            total++;

            current = nand_op(current, current);
        }

        std::cout << "  ψ accuracy: " << psi_correct << "/" << total << " = "
                  << (100.0 * psi_correct / total) << "%\n";
        std::cout << "  φ accuracy: " << phi_correct << "/" << total << " = "
                  << (100.0 * phi_correct / total) << "%\n\n";
    }

    // ============================================
    // KEY QUESTION: Ang ψ ba ay laging mas mahusay?
    // ============================================
    std::cout << "KEY QUESTION: Ang ψ ba ay laging mas mahusay?\n";
    std::cout << "---------------------------------------------\n";

    auto current = encrypt(1, NTL::to_ZZ(1));
    std::cout << "  Depth | φ correct? | ψ correct? | ψ-φ gap\n";
    std::cout << "  ------|------------|------------|---------\n";

    for (int i = 0; i <= 30; i++) {
        NTL::ZZ dec_phi = decode_phi(current);
        NTL::ZZ dec_psi = decode_psi(current);

        int phi_dec = (dist(dec_phi, NTL::to_ZZ(1)) < dist(dec_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(dec_psi, offset) < dist(dec_psi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (i % 2 == 0) ? 1 : 0;

        bool phi_ok = (phi_dec == expected);
        bool psi_ok = (psi_dec == expected);

        std::cout << "  " << i << "     | " << (phi_ok ? "YES" : "NO")
                  << "         | " << (psi_ok ? "YES" : "NO")
                  << "        | "
                  << (psi_ok == phi_ok ? "same" : (psi_ok ? "ψ better" : "φ better"))
                  << "\n";

        current = nand_op(current, current);
    }

    return 0;
}
