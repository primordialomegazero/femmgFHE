// ============================================
// φ-RULE 110 FRACTIONS — 0 AT 1 BILANG φ-FRACTIONS
//
// Sa halip na 0→0, 1→1, gamitin:
// 0 → φ⁻² = 0.382
// 1 → φ⁻¹ = 0.618
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 FRACTIONS\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double PHI_INV2 = PHI_INV * PHI_INV;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // FRACTION ENCODING
    // ============================================

    cout << "========================================\n";
    cout << "  FRACTION ENCODING\n";
    cout << "========================================\n\n";

    cout << "  0 → φ⁻² = " << PHI_INV2 << "\n";
    cout << "  1 → φ⁻¹ = " << PHI_INV << "\n\n";

    // ============================================
    // TRANSITION TEST — SIMPLE SUM
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TEST — SIMPLE SUM\n";
    cout << "========================================\n\n";

    cout << "  L C R | Sum      | Fraction | Output?\n";
    cout << "  ------|----------|----------|--------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = L ? PHI_INV : PHI_INV2;
                double c_val = C ? PHI_INV : PHI_INV2;
                double r_val = R ? PHI_INV : PHI_INV2;
                
                double sum = l_val + c_val + r_val;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(8) << fixed << setprecision(4) << (sum - floor(sum)) << " | "
                     << setw(6) << rule110[(L << 2) | (C << 1) | R] << "\n";
            }
        }
    }

    // ============================================
    // WEIGHTED FRACTION ENCODING
    // ============================================

    cout << "\n========================================\n";
    cout << "  WEIGHTED FRACTION ENCODING\n";
    cout << "========================================\n\n";

    // Subukan: L→φ⁻¹, C→φ⁻², R→φ⁻³
    // Para ang bawat position ay may ibang weight

    double W_L_frac = PHI_INV;      // φ⁻¹
    double W_C_frac = PHI_INV2;     // φ⁻²
    double W_R_frac = PHI_INV2 * PHI_INV;  // φ⁻³

    cout << "  Weights: L→φ⁻¹=" << W_L_frac << ", C→φ⁻²=" << W_C_frac << ", R→φ⁻³=" << W_R_frac << "\n\n";

    cout << "  L C R | Sum      | Floor | mod 2 | Expected | Match?\n";
    cout << "  ------|----------|-------|-------|----------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = L * W_L_frac + C * W_C_frac + R * W_R_frac;
                int floor_val = (int)floor(sum);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // IBA'T IBANG FRACTION WEIGHTS
    // ============================================

    cout << "========================================\n";
    cout << "  SEARCH: PERFECT FRACTION WEIGHTS\n";
    cout << "========================================\n\n";

    vector<double> fractions = {
        PHI_INV,          // φ⁻¹ = 0.618
        PHI_INV2,         // φ⁻² = 0.382
        PHI_INV * PHI_INV2,  // φ⁻³ = 0.236
        pow(PHI, -4),     // φ⁻⁴ = 0.146
        pow(PHI, -5),     // φ⁻⁵ = 0.090
        1.0 - PHI_INV,    // 1 - φ⁻¹ = 0.382
        PHI_INV - PHI_INV2, // φ⁻¹ - φ⁻² = 0.236
        PHI_INV + PHI_INV2, // φ⁻¹ + φ⁻² = 1.0
    };

    int best_match = 0;
    double best_a = 0, best_b = 0, best_c = 0;

    for (double a : fractions) {
        for (double b : fractions) {
            for (double c : fractions) {
                int match = 0;
                for (int L : {0, 1}) {
                    for (int C : {0, 1}) {
                        for (int R : {0, 1}) {
                            double sum = L * a + C * b + R * c;
                            int floor_val = (int)floor(sum);
                            int mod2 = floor_val % 2;
                            int expected = rule110[(L << 2) | (C << 1) | R];
                            if (mod2 == expected) match++;
                        }
                    }
                }
                if (match > best_match) {
                    best_match = match;
                    best_a = a;
                    best_b = b;
                    best_c = c;
                }
            }
        }
    }

    cout << "  Best: " << best_match << "/8\n";
    cout << "  α=" << best_a << " (φ^" << log(best_a) / log(PHI) << ")\n";
    cout << "  β=" << best_b << " (φ^" << log(best_b) / log(PHI) << ")\n";
    cout << "  γ=" << best_c << " (φ^" << log(best_c) / log(PHI) << ")\n\n";

    if (best_match == 8) {
        cout << "  ✅ PERFECT FRACTION WEIGHTS!\n";
        cout << "  L C R | Sum      | Floor | mod 2 | Expected\n";
        cout << "  ------|----------|-------|-------|----------\n";
        for (int L : {0, 1}) {
            for (int C : {0, 1}) {
                for (int R : {0, 1}) {
                    double sum = L * best_a + C * best_b + R * best_c;
                    int floor_val = (int)floor(sum);
                    int mod2 = floor_val % 2;
                    int expected = rule110[(L << 2) | (C << 1) | R];
                    cout << "  " << L << " " << C << " " << R << " | "
                         << setw(8) << fixed << setprecision(4) << sum << " | "
                         << setw(5) << floor_val << " | "
                         << setw(5) << mod2 << " | "
                         << setw(8) << expected << " | ✅\n";
                }
            }
        }
    }

    return 0;
}
