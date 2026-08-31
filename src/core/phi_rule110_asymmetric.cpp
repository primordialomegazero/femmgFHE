// ============================================
// φ-RULE 110 ASYMMETRIC — GOLDEN THRESHOLD
//
// Asymmetric threshold para sa 8/8:
// output = 1 kung sum ∈ (φ¹ - δ₁, φ¹ + δ₂)
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 ASYMMETRIC\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);

    // ============================================
    // HANAPIN ANG PERFECT ASYMMETRIC THRESHOLD
    // ============================================

    cout << "========================================\n";
    cout << "  SEARCH: PERFECT ASYMMETRIC THRESHOLD\n";
    cout << "========================================\n\n";

    // Hanapin ang δ₁ at δ₂ na nagbibigay ng 8/8
    double best_d1 = 0, best_d2 = 0;
    int best_match = 0;

    for (double d1 = 0.1; d1 <= 1.5; d1 += 0.001) {
        for (double d2 = 0.1; d2 <= 1.5; d2 += 0.001) {
            int match = 0;
            for (int L : {0, 1}) {
                for (int C : {0, 1}) {
                    for (int R : {0, 1}) {
                        double l_val = (L ? V_ONE : V_ZERO) * PHI;
                        double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                        double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                        double sum = l_val + c_val + r_val;
                        
                        int expected = rule110[(L << 2) | (C << 1) | R];
                        int output = (sum >= PHI - d1 && sum <= PHI + d2) ? 1 : 0;
                        
                        if (output == expected) match++;
                    }
                }
            }
            if (match > best_match) {
                best_match = match;
                best_d1 = d1;
                best_d2 = d2;
            }
            if (match == 8) break;
        }
        if (best_match == 8) break;
    }

    cout << "  Best: " << best_match << "/8\n";
    cout << "  δ₁ = " << best_d1 << " (lower bound)\n";
    cout << "  δ₂ = " << best_d2 << " (upper bound)\n\n";

    if (best_match == 8) {
        cout << "  ✅ PERFECT ASYMMETRIC THRESHOLD!\n\n";
        cout << "  Band: [" << PHI - best_d1 << ", " << PHI + best_d2 << "]\n\n";
        
        cout << "  L C R | Sum      | In band? | Output | Expected\n";
        cout << "  ------|----------|----------|--------|----------\n";
        
        for (int L : {0, 1}) {
            for (int C : {0, 1}) {
                for (int R : {0, 1}) {
                    double l_val = (L ? V_ONE : V_ZERO) * PHI;
                    double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                    double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                    double sum = l_val + c_val + r_val;
                    
                    int expected = rule110[(L << 2) | (C << 1) | R];
                    bool in_band = (sum >= PHI - best_d1 && sum <= PHI + best_d2);
                    int output = in_band ? 1 : 0;
                    
                    cout << "  " << L << " " << C << " " << R << " | "
                         << setw(8) << fixed << setprecision(4) << sum << " | "
                         << setw(8) << (in_band ? "YES" : "NO") << " | "
                         << setw(6) << output << " | "
                         << setw(8) << expected << " | ✅\n";
                }
            }
        }
        
        // Hanapin ang φ-relation ng δ₁ at δ₂
        cout << "\n  φ-relations:\n";
        cout << "  δ₁ = " << best_d1 << " ≈ φ^" << log(best_d1) / log(PHI) << "\n";
        cout << "  δ₂ = " << best_d2 << " ≈ φ^" << log(best_d2) / log(PHI) << "\n";
        cout << "  δ₁/δ₂ = " << best_d1 / best_d2 << " ≈ φ^" << log(best_d1 / best_d2) / log(PHI) << "\n";
    }

    return 0;
}
