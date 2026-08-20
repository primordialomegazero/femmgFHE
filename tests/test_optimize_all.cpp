// OPTIMIZE ALL — Lahat ng Direction
// 1. Hanapin ang optimal transition depth para sa adaptive switching
// 2. Error pattern analysis — Fibonacci ba talaga?
// 3. Iba't ibang k values — may mas mahusay ba?

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  OPTIMIZE ALL\n";
    std::cout << "  1. Optimal transition depth\n";
    std::cout << "  2. Error pattern analysis\n";
    std::cout << "  3. Different k values\n";
    std::cout << "========================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    auto dist = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ d = (a > b) ? a - b : b - a;
        if (d > Q/2) d = Q - d;
        return d;
    };

    // ============================================
    // TEST 1: OPTIMAL TRANSITION DEPTH
    // ============================================
    std::cout << "TEST 1: OPTIMAL TRANSITION DEPTH\n";
    std::cout << "==================================\n";
    std::cout << "  I-scan ang lahat ng transition depths\n\n";

    for (int k_val : {30, 42, 50, 60}) {
        NTL::ZZ phi_k = NTL::to_ZZ(1);
        NTL::ZZ psi_k = NTL::to_ZZ(1);
        for (int i = 0; i < k_val; i++) {
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

        std::cout << "  k=" << k_val << ":\n";

        // Hanapin ang best transition depth
        int best_transition = -1;
        int best_accuracy = 0;

        for (int trans = 0; trans <= 30; trans++) {
            auto current = encrypt(1, NTL::to_ZZ(1));
            int correct = 0;
            int total = 0;

            for (int depth = 0; depth <= 40; depth++) {
                NTL::ZZ d_phi = decode_phi(current);
                NTL::ZZ d_psi = decode_psi(current);

                int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
                int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;

                int decision = (depth <= trans) ? phi_dec : psi_dec;
                int expected = (depth % 2 == 0) ? 1 : 0;

                if (decision == expected) correct++;
                total++;

                current = nand_op(current, current);
            }

            if (correct > best_accuracy) {
                best_accuracy = correct;
                best_transition = trans;
            }
        }

        std::cout << "    Best transition: depth " << best_transition
                  << " (accuracy: " << (100.0 * best_accuracy / 41) << "%)\n\n";
    }

    // ============================================
    // TEST 2: ERROR PATTERN ANALYSIS
    // ============================================
    std::cout << "TEST 2: ERROR PATTERN ANALYSIS\n";
    std::cout << "================================\n";
    std::cout << "  Ang errors ba ay Fibonacci-spaced?\n\n";

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

    auto current = encrypt(1, NTL::to_ZZ(1));
    std::vector<int> error_depths;

    for (int depth = 0; depth <= 50; depth++) {
        NTL::ZZ d_psi = decode_psi(current);
        int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (depth % 2 == 0) ? 1 : 0;

        if (psi_dec != expected) {
            error_depths.push_back(depth);
        }

        current = nand_op(current, current);
    }

    std::cout << "  ψ error depths: ";
    for (int d : error_depths) std::cout << d << " ";
    std::cout << "\n\n";

    // Gaps between errors
    if (error_depths.size() > 1) {
        std::cout << "  Gaps: ";
        for (int i = 1; i < error_depths.size(); i++) {
            std::cout << error_depths[i] - error_depths[i-1] << " ";
        }
        std::cout << "\n\n";
    }

    // ============================================
    // TEST 3: DIFFERENT K VALUES
    // ============================================
    std::cout << "TEST 3: DIFFERENT K VALUES\n";
    std::cout << "============================\n";
    std::cout << "  Accuracy para sa iba't ibang k\n\n";

    for (int k_val : {20, 30, 42, 50, 60, 80, 100}) {
        NTL::ZZ pk = NTL::to_ZZ(1);
        NTL::ZZ sk = NTL::to_ZZ(1);
        for (int i = 0; i < k_val; i++) {
            pk = (pk * phi) % Q;
            sk = (sk * psi) % Q;
        }
        NTL::ZZ ipk = NTL::InvMod(pk, Q);
        NTL::ZZ isk = NTL::InvMod(sk, Q);
        NTL::ZZ off = (pk * isk) % Q;

        auto enc = [&](int m, NTL::ZZ e) {
            return (NTL::to_ZZ(m) * pk + e * sk) % Q;
        };
        auto dec_p = [&](NTL::ZZ ct) {
            return (ct * ipk) % Q;
        };
        auto dec_s = [&](NTL::ZZ ct) {
            return (ct * isk) % Q;
        };
        auto nand = [&](NTL::ZZ a, NTL::ZZ b) {
            NTL::ZZ prod = (a * b) % Q;
            NTL::ZZ scaled = (prod * ipk) % Q;
            NTL::ZZ result = (pk - scaled) % Q;
            if (result < 0) result += Q;
            return result;
        };

        auto curr = enc(1, NTL::to_ZZ(1));
        int psi_ok = 0, phi_ok = 0;
        int total = 30;

        for (int depth = 0; depth < total; depth++) {
            int psi_d = (dist(dec_s(curr), off) < dist(dec_s(curr), NTL::to_ZZ(0))) ? 1 : 0;
            int phi_d = (dist(dec_p(curr), NTL::to_ZZ(1)) < dist(dec_p(curr), NTL::to_ZZ(0))) ? 1 : 0;
            int exp = (depth % 2 == 0) ? 1 : 0;

            if (psi_d == exp) psi_ok++;
            if (phi_d == exp) phi_ok++;

            curr = nand(curr, curr);
        }

        std::cout << "  k=" << k_val << ": ψ=" << (100.0 * psi_ok / total)
                  << "% φ=" << (100.0 * phi_ok / total) << "%\n";
    }

    return 0;
}
