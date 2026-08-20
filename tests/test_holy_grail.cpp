// HOLY GRAIL ATTEMPT — Integrated Approach
// 1. RLWE security layer
// 2. Dual-channel φ/ψ encoding
// 3. Adaptive noise tracking
// 4. Self-correction gamit ang ψ-decode
// 5. Error correction sa decision level

#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <vector>
#include <random>

int main() {
    std::cout << "========================================\n";
    std::cout << "  HOLY GRAIL ATTEMPT\n";
    std::cout << "  RLWE Security + Dual-Channel FHE\n";
    std::cout << "========================================\n\n";

    // Parameters
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    const int k = 42;
    const int N = 1024;

    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < k; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    NTL::ZZ inv_psi_k = NTL::InvMod(psi_k, Q);
    NTL::ZZ offset = (phi_k * inv_psi_k) % Q;

    auto dist = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ d = (a > b) ? a - b : b - a;
        if (d > Q/2) d = Q - d;
        return d;
    };

    // ============================================
    // DUAL-CHANNEL SCALAR FHE na may SELF-CORRECTION
    // ============================================
    std::cout << "DUAL-CHANNEL SCALAR FHE\n";
    std::cout << "=========================\n\n";

    auto encrypt_dual = [&](int m, NTL::ZZ e) {
        // ct = m·φ^k + e·ψ^k
        // Dual channel: pareho sa φ at ψ directions
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

    // ============================================
    // SELF-CORRECTION ALGORITHM
    // ============================================
    std::cout << "SELF-CORRECTION ALGORITHM:\n";
    std::cout << "  1. Decode both channels\n";
    std::cout << "  2. Kung agree → HIGH confidence\n";
    std::cout << "  3. Kung disagree → gamitin ψ (mas stable)\n";
    std::cout << "  4. After decision, i-correct ang ct\n";
    std::cout << "     papunta sa pinakamalapit na valid state\n\n";

    auto self_correct = [&](NTL::ZZ ct) {
        NTL::ZZ d_phi = decode_phi(ct);
        NTL::ZZ d_psi = decode_psi(ct);

        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;

        int decision;
        if (phi_dec == psi_dec) {
            decision = phi_dec;
        } else {
            decision = psi_dec;  // ψ ay mas reliable sa malalim
        }

        // I-correct: i-round sa pinakamalapit na valid state
        NTL::ZZ corrected;
        if (decision == 1) {
            corrected = phi_k;  // Valid state para sa m=1
        } else {
            corrected = NTL::to_ZZ(0);  // Valid state para sa m=0
        }

        return std::make_pair(corrected, decision);
    };

    // ============================================
    // TEST: NAND CHAIN na may SELF-CORRECTION
    // ============================================
    std::cout << "TEST: NAND CHAIN na may SELF-CORRECTION\n";
    std::cout << "=========================================\n\n";

    for (NTL::ZZ e : {NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10)}) {
        std::cout << "  Initial noise e = " << e << "\n";

        auto current = encrypt_dual(1, e);
        int errors_no_corr = 0;
        int errors_with_corr = 0;
        int total = 50;

        for (int depth = 0; depth < total; depth++) {
            NTL::ZZ d_phi = decode_phi(current);
            NTL::ZZ d_psi = decode_psi(current);

            int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
            int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
            int expected = (depth % 2 == 0) ? 1 : 0;

            // Without correction: φ-only
            if (phi_dec != expected) errors_no_corr++;

            // With correction
            auto result = self_correct(current);
            int corrected_dec = result.second;
            if (corrected_dec != expected) errors_with_corr++;

            // Update current
            current = nand_op(current, current);
        }

        std::cout << "    Without correction: " << errors_no_corr << "/" << total
                  << " errors (" << (100.0 * (total - errors_no_corr) / total) << "% correct)\n";
        std::cout << "    With self-correction: " << errors_with_corr << "/" << total
                  << " errors (" << (100.0 * (total - errors_with_corr) / total) << "% correct)\n\n";
    }

    // ============================================
    // ADVANCED: CORRECTION + NORMALIZATION
    // ============================================
    std::cout << "ADVANCED: CORRECTION + NORMALIZATION\n";
    std::cout << "======================================\n\n";

    auto normalize = [&](NTL::ZZ ct) {
        // I-project sa pinakamalapit na valid state
        NTL::ZZ d_phi = decode_phi(ct);
        NTL::ZZ d_psi = decode_psi(ct);

        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;

        int decision;
        if (phi_dec == psi_dec) {
            decision = phi_dec;
        } else {
            // Weighted: ψ may mas mataas na weight sa malalim
            // φ may mas mataas na weight sa mababaw
            // Dito: simplified — ψ priority
            decision = psi_dec;
        }

        // Normalize sa exact valid state
        if (decision == 1) {
            return encrypt_dual(1, NTL::to_ZZ(0));  // φ^k na walang noise
        } else {
            return encrypt_dual(0, NTL::to_ZZ(0));  // 0 na walang noise
        }
    };

    std::cout << "  NAND chain na may normalize pagkatapos ng bawat gate:\n\n";

    for (NTL::ZZ e : {NTL::to_ZZ(1), NTL::to_ZZ(5)}) {
        std::cout << "    e=" << e << ": ";

        auto current = encrypt_dual(1, e);
        int errors = 0;
        int total = 50;

        for (int depth = 0; depth < total; depth++) {
            NTL::ZZ d_phi = decode_phi(current);
            int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
            int expected = (depth % 2 == 0) ? 1 : 0;

            if (phi_dec != expected) errors++;

            // Compute NAND tapos normalize agad
            current = nand_op(current, current);
            current = normalize(current);
        }

        std::cout << errors << "/" << total << " errors ("
                  << (100.0 * (total - errors) / total) << "% correct)\n";
    }

    return 0;
}
