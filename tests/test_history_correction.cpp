// HISTORY-BASED CORRECTION
// 1. Mag-accumulate ng decisions sa bawat gate
// 2. I-detect ang error pattern
// 3. I-correct periodically (hindi every gate)
// 4. Gumamit ng Fibonacci/Lucas periodicity

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  HISTORY-BASED CORRECTION\n";
    std::cout << "  Accumulate + Periodic Correction\n";
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
    // TEST 1: ERROR PATTERN COLLECTION
    // ============================================
    std::cout << "TEST 1: ERROR PATTERN COLLECTION\n";
    std::cout << "==================================\n\n";

    auto current = encrypt(1, NTL::to_ZZ(1));
    std::vector<int> phi_decisions;
    std::vector<int> psi_decisions;
    std::vector<int> actual_errors;
    std::vector<int> expected_values;

    for (int depth = 0; depth <= 50; depth++) {
        NTL::ZZ d_phi = decode_phi(current);
        NTL::ZZ d_psi = decode_psi(current);

        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (depth % 2 == 0) ? 1 : 0;

        phi_decisions.push_back(phi_dec);
        psi_decisions.push_back(psi_dec);
        actual_errors.push_back(phi_dec != expected ? 1 : 0);
        expected_values.push_back(expected);

        current = nand_op(current, current);
    }

    // Error pattern
    std::cout << "  Error positions (φ): ";
    for (int i = 0; i < actual_errors.size(); i++) {
        if (actual_errors[i]) std::cout << i << " ";
    }
    std::cout << "\n\n";

    // ψ disagreement positions
    std::cout << "  Disagreement positions (φ≠ψ): ";
    for (int i = 0; i < phi_decisions.size(); i++) {
        if (phi_decisions[i] != psi_decisions[i]) std::cout << i << " ";
    }
    std::cout << "\n\n";

    // ============================================
    // TEST 2: MAJORITY OVER WINDOW
    // ============================================
    std::cout << "TEST 2: MAJORITY OVER WINDOW\n";
    std::cout << "==============================\n\n";

    // Kung ang actual value ay period-2 (0,1,0,1,...),
    // pwede nating gamitin ang window majority para mag-correct

    std::cout << "  Window size | Accuracy\n";
    std::cout << "  ------------|----------\n";

    for (int window = 1; window <= 10; window++) {
        int correct = 0;
        int total = 0;

        for (int i = window; i < phi_decisions.size() - window; i++) {
            // Majority sa window
            int votes_1 = 0, votes_0 = 0;
            for (int j = i - window; j <= i + window; j++) {
                if (phi_decisions[j] == 1) votes_1++;
                else votes_0++;
            }
            int majority = (votes_1 > votes_0) ? 1 : 0;
            int expected = expected_values[i];

            if (majority == expected) correct++;
            total++;
        }

        std::cout << "  " << window << "          | " 
                  << (100.0 * correct / total) << "%\n";
    }

    // ============================================
    // TEST 3: PERIODIC CORRECTION
    // ============================================
    std::cout << "\nTEST 3: PERIODIC CORRECTION\n";
    std::cout << "=============================\n\n";

    // I-correct every N gates, hindi every gate

    std::cout << "  Correct every | Accuracy\n";
    std::cout << "  --------------|----------\n";

    for (int period = 1; period <= 20; period++) {
        auto test_current = encrypt(1, NTL::to_ZZ(1));
        int errors = 0;
        int total = 50;
        int gates_since_correction = 0;

        for (int depth = 0; depth < total; depth++) {
            NTL::ZZ d_phi = decode_phi(test_current);
            int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
            int expected = (depth % 2 == 0) ? 1 : 0;

            if (phi_dec != expected) errors++;

            test_current = nand_op(test_current, test_current);

            gates_since_correction++;
            if (gates_since_correction >= period) {
                // I-correct gamit ang ψ-decision
                NTL::ZZ d_psi = decode_psi(test_current);
                int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
                test_current = encrypt(psi_dec, NTL::to_ZZ(1));
                gates_since_correction = 0;
            }
        }

        std::cout << "  " << period << "             | " 
                  << (100.0 * (total - errors) / total) << "%\n";
    }

    // ============================================
    // TEST 4: PREDICTIVE CORRECTION
    // ============================================
    std::cout << "\nTEST 4: PREDICTIVE CORRECTION\n";
    std::cout << "================================\n\n";

    // Gamitin ang period-2 na property:
    // expected[0] = 1, expected[1] = 0, expected[2] = 1, ...
    // Kung ang φ-decision ay hindi sumusunod sa period-2,
    // malamang error ito — i-flip ang decision

    auto test_current = encrypt(1, NTL::to_ZZ(1));
    int errors_no_predict = 0;
    int errors_with_predict = 0;
    int total = 50;

    std::vector<int> decision_history;

    for (int depth = 0; depth < total; depth++) {
        NTL::ZZ d_phi = decode_phi(test_current);
        NTL::ZZ d_psi = decode_psi(test_current);
        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (depth % 2 == 0) ? 1 : 0;

        // Without prediction
        if (phi_dec != expected) errors_no_predict++;

        // With prediction: check period-2 consistency
        int predicted = (depth % 2 == 0) ? 1 : 0;

        // Kung may disagreement, gamitin ang period-2 prediction
        int final_dec;
        if (phi_dec != psi_dec) {
            final_dec = predicted;  // Trust the period-2
        } else {
            final_dec = phi_dec;
        }

        if (final_dec != expected) errors_with_predict++;

        decision_history.push_back(final_dec);
        test_current = nand_op(test_current, test_current);
    }

    std::cout << "  Without prediction: " << errors_no_predict << "/" << total << " errors ("
              << (100.0 * (total - errors_no_predict) / total) << "%)\n";
    std::cout << "  With period-2 prediction: " << errors_with_predict << "/" << total << " errors ("
              << (100.0 * (total - errors_with_predict) / total) << "%)\n";

    // ============================================
    // TEST 5: FIBONACCI CORRECTION
    // ============================================
    std::cout << "\nTEST 5: FIBONACCI CORRECTION\n";
    std::cout << "===============================\n\n";

    // I-correct sa Fibonacci-spaced intervals
    std::vector<int> fib_intervals = {1, 2, 3, 5, 8, 13, 21, 34};
    
    auto test_curr = encrypt(1, NTL::to_ZZ(1));
    int errors_fib = 0;
    int total_fib = 50;
    int next_correction = fib_intervals[0];
    int fib_idx = 0;

    for (int depth = 0; depth < total_fib; depth++) {
        NTL::ZZ d_phi = decode_phi(test_curr);
        int phi_dec = (dist(d_phi, NTL::to_ZZ(1)) < dist(d_phi, NTL::to_ZZ(0))) ? 1 : 0;
        int expected = (depth % 2 == 0) ? 1 : 0;

        if (phi_dec != expected) errors_fib++;

        test_curr = nand_op(test_curr, test_curr);

        if (depth == next_correction) {
            NTL::ZZ d_psi = decode_psi(test_curr);
            int psi_dec = (dist(d_psi, offset) < dist(d_psi, NTL::to_ZZ(0))) ? 1 : 0;
            test_curr = encrypt(psi_dec, NTL::to_ZZ(1));
            
            fib_idx++;
            if (fib_idx < fib_intervals.size()) {
                next_correction += fib_intervals[fib_idx];
            }
        }
    }

    std::cout << "  Fibonacci-spaced correction: " << errors_fib << "/" << total_fib 
              << " errors (" << (100.0 * (total_fib - errors_fib) / total_fib) << "%)\n";

    return 0;
}
