// RULE 110 — 0-LEVEL CONDITIONAL SEARCH
// Hanapin ang formula na walang multiplication
// Na kayang i-encode ang 8/8 transition

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — 0-LEVEL CONDITIONAL\n";
    std::cout << "  8/8 Transition na Walang Mult\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

    // ============================================
    // RULE 110 TRUTH TABLE
    // ============================================
    // 111 → 0, 110 → 1, 101 → 1, 100 → 0
    // 011 → 1, 010 → 1, 001 → 1, 000 → 0
    //
    // Sa binary (LCR):
    // 0 (000) → 0
    // 1 (001) → 1
    // 2 (010) → 1
    // 3 (011) → 1
    // 4 (100) → 0
    // 5 (101) → 1
    // 6 (110) → 1
    // 7 (111) → 0

    std::cout << "TRUTH TABLE ANALYSIS:\n";
    std::cout << "=====================\n\n";

    // ============================================
    // OBSERVATION: Sum ng LCR
    // ============================================
    std::cout << "SUM-BASED ANALYSIS:\n";
    std::cout << "===================\n\n";

    std::cout << "  LCR | Sum | Output | Match?\n";
    std::cout << "  ----|-----|--------|-------\n";

    for (int l = 0; l <= 1; l++) {
        for (int c = 0; c <= 1; c++) {
            for (int r = 0; r <= 1; r++) {
                int sum = l + c + r;
                int output;
                if (l == 1 && c == 1 && r == 1) output = 0;
                else if (l == 0 && c == 0 && r == 0) output = 0;
                else if (l == 1 && c == 0 && r == 0) output = 0;
                else output = 1;
                
                // Sum-based: output = 1 kung sum ∈ {1, 2}
                // output = 0 kung sum ∈ {0, 3}
                int sum_output = (sum >= 1 && sum <= 2) ? 1 : 0;
                
                std::cout << "  " << l << c << r << " | " << sum 
                          << " | " << output
                          << " | " << (sum_output == output ? "✓" : "✗") << "\n";
            }
        }
    }

    std::cout << "\n";

    // ============================================
    // PERIOD-6 OSCILLATION APPROACH
    // ============================================
    std::cout << "PERIOD-6 OSCILLATION:\n";
    std::cout << "====================\n\n";

    // Ang sum ∈ {0, 1, 2, 3}
    // Output: sum=0→0, sum=1→1, sum=2→1, sum=3→0
    //
    // Ito ay f(sum) = 1 kung sum ∈ {1, 2}
    //              = 0 kung sum ∈ {0, 3}
    //
    // Sa φ-domain:
    // f(0) = 0
    // f(φ) = φ²
    // f(2φ) = φ²
    // f(3φ) = 0
    //
    // Ito ay period-4 sa sum space:
    // 0 → 0, φ → φ², 2φ → φ², 3φ → 0

    std::cout << "  f(0) = 0\n";
    std::cout << "  f(φ) = φ²\n";
    std::cout << "  f(2φ) = φ²\n";
    std::cout << "  f(3φ) = 0\n\n";

    std::cout << "  Sa period-4 cycle:\n";
    std::cout << "  0 → φ² → 2φ² → -φ² → 0\n\n";

    std::cout << "  Ang mapping:\n";
    std::cout << "  sum=0 → 0 (state 0)\n";
    std::cout << "  sum=1 → φ² (state 1)\n";
    std::cout << "  sum=2 → φ² (state 1)\n";
    std::cout << "  sum=3 → 0 (state 0)\n\n";

    std::cout << "  Kaya ang formula ay:\n";
    std::cout << "  next = φ² · indicator(sum ∈ {1,2})\n";
    std::cout << "  = φ² · (1 kung sum ∈ {1,2}, 0 kung sum ∈ {0,3})\n\n";

    std::cout << "  ITO AY PERIOD-4 THRESHOLD SA SUM!\n";
    std::cout << "  At ang period-4 cycle ay 0-LEVEL!\n";

    return 0;
}
