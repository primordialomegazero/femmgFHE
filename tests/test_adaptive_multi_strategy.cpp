// ADAPTIVE MULTI-STRATEGY — LAHAT NG DIRECTION
// 1. Adaptive switching: ψ sa malalim, φ sa mababaw
// 2. Multi-level encoding: mas maraming channels
// 3. Error correction: weighted voting φ+ψ

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  ADAPTIVE MULTI-STRATEGY\n";
    std::cout << "  Lahat ng Direction Sabay-sabay\n";
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

    // ψ^(2k) at φ^(2k) para sa higher-order channels
    NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
    NTL::ZZ psi_2k = (psi_k * psi_k) % Q;
    NTL::ZZ inv_phi_2k = NTL::InvMod(phi_2k, Q);
    NTL::ZZ inv_psi_2k = NTL::InvMod(psi_2k, Q);
    NTL::ZZ offset_2k = (phi_2k * inv_psi_2k) % Q;

    auto encrypt = [&](int m, NTL::ZZ e) {
        return (NTL::to_ZZ(m) * phi_k + e * psi_k) % Q;
    };
    auto decode_phi = [&](NTL::ZZ ct) {
        return (ct * inv_phi_k) % Q;
    };
    auto decode_psi = [&](NTL::ZZ ct) {
        return (ct * inv_psi_k) % Q;
    };
    auto decode_phi_2k = [&](NTL::ZZ ct) {
        return (ct * inv_phi_2k) % Q;
    };
    auto decode_psi_2k = [&](NTL::ZZ ct) {
        return (ct * inv_psi_2k) % Q;
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
    // STRATEGY 1: ADAPTIVE SWITCHING
    // ============================================
    std::cout << "STRATEGY 1: ADAPTIVE SWITCHING\n";
    std::cout << "================================\n";
    std::cout << "  φ sa depth 0-5, ψ sa depth 6+\n\n";

    auto current = encrypt(1, NTL::to_ZZ(1));
    int adaptive_correct = 0;
    int phi_correct = 0;
    int psi_correct = 0;
    int total = 0;

    for (int i = 0; i <= 40; i++) {
        NTL::ZZ dec_phi = decode_phi(current);
        NTL::ZZ dec_psi = decode_psi(current);

        int phi_dec = (dist(dec_phi, NTL::to_ZZ(1)) < dist(dec_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(dec_psi, offset) < dist(dec_psi, NTL::to_ZZ(0))) ? 1 : 0;

        // Adaptive: φ para sa shallow, ψ para sa deep
        int adaptive_dec = (i <= 5) ? phi_dec : psi_dec;

        int expected = (i % 2 == 0) ? 1 : 0;
        if (adaptive_dec == expected) adaptive_correct++;
        if (phi_dec == expected) phi_correct++;
        if (psi_dec == expected) psi_correct++;
        total++;

        current = nand_op(current, current);
    }

    std::cout << "  Adaptive: " << adaptive_correct << "/" << total << " = "
              << (100.0 * adaptive_correct / total) << "%\n";
    std::cout << "  φ-only: " << phi_correct << "/" << total << " = "
              << (100.0 * phi_correct / total) << "%\n";
    std::cout << "  ψ-only: " << psi_correct << "/" << total << " = "
              << (100.0 * psi_correct / total) << "%\n\n";

    // ============================================
    // STRATEGY 2: MULTI-LEVEL ENCODING (4 channels)
    // ============================================
    std::cout << "STRATEGY 2: MULTI-LEVEL ENCODING (4 channels)\n";
    std::cout << "===============================================\n";
    std::cout << "  Channels: φ^k, ψ^k, φ^(2k), ψ^(2k)\n\n";

    current = encrypt(1, NTL::to_ZZ(1));
    int multi_correct = 0;

    std::cout << "  Depth | φ | ψ | φ2 | ψ2 | Weighted | Expected\n";
    std::cout << "  ------|---|---|----|----|----------|----------\n";

    for (int i = 0; i <= 20; i++) {
        NTL::ZZ d_phi = decode_phi(current);
        NTL::ZZ d_psi = decode_psi(current);
        NTL::ZZ d_phi2 = decode_phi_2k(current);
        NTL::ZZ d_psi2 = decode_psi_2k(current);

        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
        int phi2_dec = (dist(d_phi2, NTL::to_ZZ(1)) < dist(d_phi2, NTL::to_ZZ(0))) ? 1 : 0;
        int psi2_dec = (dist(d_psi2, offset_2k) < dist(d_psi2, NTL::to_ZZ(0))) ? 1 : 0;

        // Weighted voting: ψ may mas mataas na weight sa malalim
        double w_phi = (i <= 5) ? 0.4 : 0.1;
        double w_psi = (i <= 5) ? 0.3 : 0.5;
        double w_phi2 = 0.15;
        double w_psi2 = 0.15;

        double score_1 = w_phi * (phi_dec == 1) + w_psi * (psi_dec == 1) +
                         w_phi2 * (phi2_dec == 1) + w_psi2 * (psi2_dec == 1);
        double score_0 = w_phi * (phi_dec == 0) + w_psi * (psi_dec == 0) +
                         w_phi2 * (phi2_dec == 0) + w_psi2 * (psi2_dec == 0);

        int weighted_dec = (score_1 > score_0) ? 1 : 0;
        int expected = (i % 2 == 0) ? 1 : 0;

        if (weighted_dec == expected) multi_correct++;

        std::cout << "  " << i << "     | " << phi_dec << " | " << psi_dec
                  << " | " << phi2_dec << "  | " << psi2_dec
                  << "  | " << weighted_dec
                  << "       | " << expected << "\n";

        current = nand_op(current, current);
    }

    std::cout << "\n  Weighted voting: " << multi_correct << "/21 = "
              << (100.0 * multi_correct / 21) << "%\n\n";

    // ============================================
    // STRATEGY 3: ERROR CORRECTION CODE
    // ============================================
    std::cout << "STRATEGY 3: MAJORITY VOTING (3 channels)\n";
    std::cout << "===========================================\n";
    std::cout << "  φ, ψ, at φ^(2k) — majority wins\n\n";

    current = encrypt(1, NTL::to_ZZ(1));
    int majority_correct = 0;

    for (int i = 0; i <= 30; i++) {
        NTL::ZZ d_phi = decode_phi(current);
        NTL::ZZ d_psi = decode_psi(current);
        NTL::ZZ d_phi2 = decode_phi_2k(current);

        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
        int phi2_dec = (dist(d_phi2, NTL::to_ZZ(1)) < dist(d_phi2, NTL::to_ZZ(0))) ? 1 : 0;

        // Majority: at least 2 out of 3
        int votes_1 = phi_dec + psi_dec + phi2_dec;
        int majority_dec = (votes_1 >= 2) ? 1 : 0;

        int expected = (i % 2 == 0) ? 1 : 0;
        if (majority_dec == expected) majority_correct++;

        current = nand_op(current, current);
    }

    std::cout << "  Majority voting: " << majority_correct << "/31 = "
              << (100.0 * majority_correct / 31) << "%\n\n";

    // ============================================
    // STRATEGY 4: CONFIDENCE-WEIGHTED
    // ============================================
    std::cout << "STRATEGY 4: CONFIDENCE-WEIGHTED\n";
    std::cout << "==================================\n";
    std::cout << "  Mas malapit sa decision boundary = mas mataas na confidence\n\n";

    current = encrypt(1, NTL::to_ZZ(1));
    int conf_correct = 0;

    for (int i = 0; i <= 30; i++) {
        NTL::ZZ d_phi = decode_phi(current);
        NTL::ZZ d_psi = decode_psi(current);

        // φ confidence: distance to 0 vs 1
        NTL::ZZ phi_dist_0 = dist(d_phi, NTL::to_ZZ(0));
        NTL::ZZ phi_dist_1 = dist(d_phi, NTL::to_ZZ(1));
        double phi_conf = (phi_dist_0 + phi_dist_1 > 0) ?
            (double)NTL::to_long((phi_dist_0 > phi_dist_1) ? phi_dist_1 : phi_dist_0) / 
            (double)NTL::to_long(phi_dist_0 + phi_dist_1) : 0.5;

        // ψ confidence
        NTL::ZZ psi_dist_0 = dist(d_psi, NTL::to_ZZ(0));
        NTL::ZZ psi_dist_off = dist(d_psi, offset);
        double psi_conf = (psi_dist_0 + psi_dist_off > 0) ?
            (double)NTL::to_long((psi_dist_0 > psi_dist_off) ? psi_dist_off : psi_dist_0) /
            (double)NTL::to_long(psi_dist_0 + psi_dist_off) : 0.5;

        // Weighted by confidence
        int phi_dec = (phi_dist_1 < phi_dist_0) ? 1 : 0;
        int psi_dec = (psi_dist_off < psi_dist_0) ? 1 : 0;

        double score_1 = phi_conf * (phi_dec == 1) + psi_conf * (psi_dec == 1);
        double score_0 = phi_conf * (phi_dec == 0) + psi_conf * (psi_dec == 0);

        int conf_dec = (score_1 > score_0) ? 1 : 0;
        int expected = (i % 2 == 0) ? 1 : 0;

        if (conf_dec == expected) conf_correct++;

        current = nand_op(current, current);
    }

    std::cout << "  Confidence-weighted: " << conf_correct << "/31 = "
              << (100.0 * conf_correct / 31) << "%\n\n";

    // ============================================
    // COMPARISON SUMMARY
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  COMPARISON SUMMARY (40 depths, e=1)\n";
    std::cout << "========================================\n";
    std::cout << "  φ-only:            " << (100.0 * phi_correct / total) << "%\n";
    std::cout << "  ψ-only:            " << (100.0 * psi_correct / total) << "%\n";
    std::cout << "  Adaptive:          " << (100.0 * adaptive_correct / total) << "%\n";
    std::cout << "  Weighted (4ch):    " << (100.0 * multi_correct / 21) << "% (21 depths)\n";
    std::cout << "  Majority (3ch):    " << (100.0 * majority_correct / 31) << "% (31 depths)\n";
    std::cout << "  Conf-weighted:     " << (100.0 * conf_correct / 31) << "% (31 depths)\n";

    return 0;
}
