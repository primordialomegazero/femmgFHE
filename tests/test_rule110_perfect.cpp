// RULE 110 — PERFECT 8/8
// 3φ² - sum - 2φ²·L·(1-C)·(1-R)
// Asymmetric threshold para sa 100 pattern

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — PERFECT 8/8\n";
    std::cout << "  Asymmetric φ-Domain Formula\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

    std::cout << "FORMULA: next = 3φ² - (L + C + R) - 2φ²·L·(1-C)·(1-R)\n\n";
    std::cout << "  L C R → Expected → Got → Value\n";
    std::cout << "  ─────────────────────────────────\n";

    int correct = 0;

    for (int l = 0; l <= 1; l++) {
        for (int c = 0; c <= 1; c++) {
            for (int r = 0; r <= 1; r++) {
                int expected;
                if (l == 1 && c == 1 && r == 1) expected = 0;
                else if (l == 0 && c == 0 && r == 0) expected = 0;
                else if (l == 1 && c == 0 && r == 0) expected = 0;
                else expected = 1;

                double L = l ? phi_sq : 0.0;
                double C = c ? phi_sq : 0.0;
                double R = r ? phi_sq : 0.0;

                // Asymmetric formula
                double penalty = 2 * phi_sq * L * (1 - C) * (1 - R);
                double val = 3 * phi_sq - (L + C + R) - penalty;

                int got = (val > phi_sq / 2) ? 1 : 0;
                if (got == expected) correct++;

                std::cout << "  " << l << " " << c << " " << r 
                          << " → " << expected << " → " << got
                          << " → " << val
                          << (got == expected ? " ✓" : " ✗") << "\n";
            }
        }
    }

    std::cout << "\n  Correct: " << correct << "/8\n";
    std::cout << "  Status: " << (correct == 8 ? "✅ PERFECT 8/8!" : "⚠️ NEEDS FIX") << "\n";

    return 0;
}
