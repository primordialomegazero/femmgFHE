// CALIBRATED NOISE REFRESH — HOLY GRAIL PATH
// 1. Hanapin ang lahat ng perfect noise levels
// 2. I-verify ang pattern (Fibonacci ba?)
// 3. I-implement ang calibrated refresh
// 4. Test sa malalim na NAND chain

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::cout << "========================================\n";
    std::cout << "  CALIBRATED NOISE REFRESH\n";
    std::cout << "  Holy Grail Path\n";
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

    auto dist = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ d = (a > b) ? a - b : b - a;
        if (d > Q/2) d = Q - d;
        return d;
    };

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

    // ============================================
    // TEST 1: COMPREHENSIVE PERFECT NOISE SCAN
    // ============================================
    std::cout << "TEST 1: PERFECT NOISE LEVELS (0-100)\n";
    std::cout << "========================================\n\n";

    std::vector<int> perfect_e;
    int max_depth_test = 30;

    for (int e_val = 0; e_val <= 100; e_val++) {
        NTL::ZZ e = NTL::to_ZZ(e_val);
        auto current = encrypt(1, e);
        int errors = 0;

        for (int depth = 0; depth <= max_depth_test; depth++) {
            NTL::ZZ d_phi = decode_phi(current);
            int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
            int expected = (depth % 2 == 0) ? 1 : 0;

            if (phi_dec != expected) errors++;
            if (errors > 0) break;

            current = nand_op(current, current);
            // Dito walang normalization — pure test lang ng noise propagation
        }

        if (errors == 0) {
            perfect_e.push_back(e_val);
        }
    }

    std::cout << "  Perfect noise levels (no errors sa " << max_depth_test << " depths):\n";
    std::cout << "  ";
    for (int e : perfect_e) std::cout << e << " ";
    std::cout << "\n\n";

    // Gaps
    if (perfect_e.size() > 1) {
        std::cout << "  Gaps: ";
        for (int i = 1; i < perfect_e.size(); i++) {
            std::cout << perfect_e[i] - perfect_e[i-1] << " ";
        }
        std::cout << "\n\n";
    }

    // ============================================
    // TEST 2: FIBONACCI / LUCAS CHECK
    // ============================================
    std::cout << "TEST 2: FIBONACCI/LUCAS CHECK\n";
    std::cout << "===============================\n\n";

    std::vector<int> fib = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89};
    std::vector<int> lucas = {2, 1, 3, 4, 7, 11, 18, 29, 47, 76};

    std::cout << "  Perfect e values na nasa Fibonacci: ";
    for (int e : perfect_e) {
        if (std::find(fib.begin(), fib.end(), e) != fib.end()) {
            std::cout << e << " ";
        }
    }
    std::cout << "\n";

    std::cout << "  Perfect e values na nasa Lucas: ";
    for (int e : perfect_e) {
        if (std::find(lucas.begin(), lucas.end(), e) != lucas.end()) {
            std::cout << e << " ";
        }
    }
    std::cout << "\n\n";

    // ============================================
    // TEST 3: CALIBRATED REFRESH
    // ============================================
    std::cout << "TEST 3: CALIBRATED REFRESH\n";
    std::cout << "===========================\n\n";

    // Strategy: Pagkatapos ng bawat NAND, i-refresh ang noise
    // sa perfect level (gamitin ang pinakamalapit na perfect e)

    int target_e = 5;  // Simulan sa e=5 na known perfect
    auto refresh_noise = [&](NTL::ZZ ct) {
        // I-decode para makuha ang decision
        NTL::ZZ d_phi = decode_phi(ct);
        NTL::ZZ d_psi = decode_psi(ct);

        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;

        int decision = (phi_dec == psi_dec) ? phi_dec : psi_dec;

        // I-refresh sa perfect noise level
        return encrypt(decision, NTL::to_ZZ(target_e));
    };

    std::cout << "  Calibrated refresh (target e=" << target_e << "):\n\n";

    for (NTL::ZZ initial_e : {NTL::to_ZZ(0), NTL::to_ZZ(1), NTL::to_ZZ(5), NTL::to_ZZ(10)}) {
        auto current = encrypt(1, initial_e);
        int errors = 0;
        int total = 50;

        for (int depth = 0; depth < total; depth++) {
            NTL::ZZ d_phi = decode_phi(current);
            int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
            int expected = (depth % 2 == 0) ? 1 : 0;

            if (phi_dec != expected) errors++;

            current = nand_op(current, current);
            current = refresh_noise(current);
        }

        std::cout << "    initial e=" << initial_e << ": " 
                  << errors << "/" << total << " errors ("
                  << (100.0 * (total - errors) / total) << "% correct)\n";
    }

    // ============================================
    // TEST 4: ADAPTIVE REFRESH — hanapin ang best target
    // ============================================
    std::cout << "\nTEST 4: ADAPTIVE REFRESH\n";
    std::cout << "=========================\n\n";

    int best_target = -1;
    int best_accuracy = 0;

    for (int target = 0; target <= 20; target++) {
        auto refresh = [&](NTL::ZZ ct) {
            NTL::ZZ d_phi = decode_phi(ct);
            NTL::ZZ d_psi = decode_psi(ct);
            int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
            int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
            int decision = (phi_dec == psi_dec) ? phi_dec : psi_dec;
            return encrypt(decision, NTL::to_ZZ(target));
        };

        auto current = encrypt(1, NTL::to_ZZ(1));
        int errors = 0;
        int total = 100;

        for (int depth = 0; depth < total; depth++) {
            NTL::ZZ d_phi = decode_phi(current);
            int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
            int expected = (depth % 2 == 0) ? 1 : 0;

            if (phi_dec != expected) errors++;

            current = nand_op(current, current);
            current = refresh(current);
        }

        int accuracy = 100 - (100 * errors / total);
        if (accuracy > best_accuracy) {
            best_accuracy = accuracy;
            best_target = target;
        }
    }

    std::cout << "  Best target e: " << best_target << " (accuracy: " << best_accuracy << "%)\n";

    return 0;
}
