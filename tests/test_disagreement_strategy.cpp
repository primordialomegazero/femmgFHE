// DISAGREEMENT STRATEGY
// Kapag φ at ψ ay nag-agree → HIGH confidence, gamitin ang decision
// Kapag φ at ψ ay nag-disagree → gamitin ang OPPOSITE ng φ

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  DISAGREEMENT STRATEGY\n";
    std::cout << "  Kapag may disagreement, i-flip ang φ\n";
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

    // ============================================
    // STRATEGY COMPARISON
    // ============================================
    std::cout << "STRATEGY COMPARISON (e=1, 20 depths):\n\n";

    auto current = encrypt(1, NTL::to_ZZ(1));
    int phi_correct = 0, psi_correct = 0, flip_correct = 0;
    int total = 0;

    std::cout << "  Depth | φ | ψ | Agree? | φ+ψ | Flip | Expected\n";
    std::cout << "  ------|---|---|--------|-----|------|----------\n";

    for (int i = 0; i <= 20; i++) {
        NTL::ZZ dec_phi = decode_phi(current);
        NTL::ZZ dec_psi = decode_psi(current);

        int phi_dec = (dist(dec_phi, NTL::to_ZZ(1)) < dist(dec_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(dec_psi, offset) < dist(dec_psi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (i % 2 == 0) ? 1 : 0;

        bool agree = (phi_dec == psi_dec);
        int agree_decision = agree ? phi_dec : phi_dec;
        int flip_decision = agree ? phi_dec : (1 - phi_dec);

        if (phi_dec == expected) phi_correct++;
        if (psi_dec == expected) psi_correct++;
        if (flip_decision == expected) flip_correct++;
        total++;

        std::cout << "  " << i << "     | " << phi_dec << " | " << psi_dec
                  << " | " << (agree ? "YES" : "NO")
                  << "    | " << agree_decision
                  << "   | " << flip_decision
                  << "    | " << expected << "\n";

        current = nand_op(current, current);
    }

    std::cout << "\n  RESULTS:\n";
    std::cout << "    φ-only: " << phi_correct << "/" << total << " = "
              << (100.0 * phi_correct / total) << "%\n";
    std::cout << "    ψ-only: " << psi_correct << "/" << total << " = "
              << (100.0 * psi_correct / total) << "%\n";
    std::cout << "    Flip strategy: " << flip_correct << "/" << total << " = "
              << (100.0 * flip_correct / total) << "%\n";

    // ============================================
    // ANALYZE ANG DISAGREEMENT PATTERN
    // ============================================
    std::cout << "\nDISAGREEMENT ANALYSIS:\n";
    std::cout << "------------------------\n";

    current = encrypt(1, NTL::to_ZZ(1));
    int disagreements = 0;
    int flip_wins = 0;

    for (int i = 0; i <= 20; i++) {
        NTL::ZZ dec_phi = decode_phi(current);
        NTL::ZZ dec_psi = decode_psi(current);

        int phi_dec = (dist(dec_phi, NTL::to_ZZ(1)) < dist(dec_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(dec_psi, offset) < dist(dec_psi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (i % 2 == 0) ? 1 : 0;

        if (phi_dec != psi_dec) {
            disagreements++;
            int flip = 1 - phi_dec;
            if (flip == expected) flip_wins++;
        }

        current = nand_op(current, current);
    }

    std::cout << "  Total disagreements: " << disagreements << "\n";
    std::cout << "  Flip wins: " << flip_wins << "\n";
    std::cout << "  Flip accuracy sa disagreement: "
              << (100.0 * flip_wins / disagreements) << "%\n";

    return 0;
}
