// ============================================
// φ-RULE 110 SPACES — IBA'T IBANG φ-SPACES
//
// Check: φ-space, φ²-space, φ-log space, Beatty space
// Hanapin kung saang space natural ang Rule 110
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
    cout << "  φ-RULE 110 SPACES\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double PHI_INV2 = PHI_INV * PHI_INV;
    const double PHI_INV3 = PHI_INV2 * PHI_INV;
    const double LN_PHI = log(PHI);

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // 1. φ-SPACE (POWERS)
    // ============================================

    cout << "========================================\n";
    cout << "  1. φ-SPACE (φ^n)\n";
    cout << "========================================\n\n";

    cout << "  Encoding: 0→1, 1→φ\n";
    cout << "  Weights: L→φ², C→φ, R→1\n\n";

    cout << "  L C R | Sum      | Floor | mod 2 | Expected | Match?\n";
    cout << "  ------|----------|-------|-------|----------|--------\n";

    int match1 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = L ? PHI * PHI : 1.0;
                double c_val = C ? PHI : 1.0;
                double r_val = R ? 1.0 : 1.0;
                
                double sum = l_val + c_val + r_val;
                int floor_val = (int)floor(sum);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match1++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(3) << sum << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "  Match: " << match1 << "/8\n\n";

    // ============================================
    // 2. φ²-SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  2. φ²-SPACE\n";
    cout << "========================================\n\n";

    cout << "  Encoding: 0→1, 1→φ²\n";
    cout << "  Weights: L→φ⁴, C→φ², R→1\n\n";

    cout << "  L C R | Sum      | Floor | mod 2 | Expected | Match?\n";
    cout << "  ------|----------|-------|-------|----------|--------\n";

    int match2 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = L ? pow(PHI, 4) : 1.0;
                double c_val = C ? PHI * PHI : 1.0;
                double r_val = R ? 1.0 : 1.0;
                
                double sum = l_val + c_val + r_val;
                int floor_val = (int)floor(sum);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match2++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(3) << sum << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "  Match: " << match2 << "/8\n\n";

    // ============================================
    // 3. φ-LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  3. φ-LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  Encoding: 0→0, 1→ln(φ)\n";
    cout << "  Weights: L→2, C→1, R→0.5\n\n";

    cout << "  L C R | Log Sum  | Exp     | Floor | mod 2 | Expected | Match?\n";
    cout << "  ------|----------|---------|-------|-------|----------|--------\n";

    int match3 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double log_sum = L * 2.0 * LN_PHI + C * 1.0 * LN_PHI + R * 0.5 * LN_PHI;
                double exp_val = exp(log_sum);
                int floor_val = (int)floor(exp_val);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match3++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(3) << log_sum << " | "
                     << setw(7) << fixed << setprecision(3) << exp_val << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "  Match: " << match3 << "/8\n\n";

    // ============================================
    // 4. NEGATIVE φ-SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  4. NEGATIVE φ-SPACE\n";
    cout << "========================================\n\n";

    cout << "  Encoding: 0→1, 1→φ⁻¹\n";
    cout << "  Weights: L→φ⁻², C→φ⁻¹, R→1\n\n";

    cout << "  L C R | Sum      | Floor | mod 2 | Expected | Match?\n";
    cout << "  ------|----------|-------|-------|----------|--------\n";

    int match4 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = L ? PHI_INV * PHI_INV : 1.0;
                double c_val = C ? PHI_INV : 1.0;
                double r_val = R ? 1.0 : 1.0;
                
                double sum = l_val + c_val + r_val;
                int floor_val = (int)floor(sum);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match4++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(3) << sum << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "  Match: " << match4 << "/8\n\n";

    // ============================================
    // 5. MIXED SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  5. MIXED SPACE\n";
    cout << "========================================\n\n";

    cout << "  Encoding: 0→φ⁻¹, 1→φ\n";
    cout << "  Weights: L→1, C→1, R→1\n\n";

    cout << "  L C R | Sum      | Floor | mod 2 | Expected | Match?\n";
    cout << "  ------|----------|-------|-------|----------|--------\n";

    int match5 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = L ? PHI : PHI_INV;
                double c_val = C ? PHI : PHI_INV;
                double r_val = R ? PHI : PHI_INV;
                
                double sum = l_val + c_val + r_val;
                int floor_val = (int)floor(sum);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match5++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(3) << sum << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "  Match: " << match5 << "/8\n\n";

    // ============================================
    // 6. EXTENDED SEARCH SA LAHAT NG SPACES
    // ============================================

    cout << "========================================\n";
    cout << "  6. EXTENDED SEARCH\n";
    cout << "========================================\n\n";

    vector<double> values = {
        0.0,
        PHI_INV3,        // φ⁻³
        PHI_INV2,        // φ⁻²
        PHI_INV,         // φ⁻¹
        0.5,             // 1/2
        1.0,             // 1
        PHI / 2.0,       // φ/2
        PHI_INV + PHI_INV2,  // φ⁻¹ + φ⁻² = 1
        PHI,             // φ
        PHI * PHI / 2.0, // φ²/2
        PHI * PHI,       // φ²
        PHI * PHI * PHI / 2.0,  // φ³/2
        PHI * PHI * PHI, // φ³
    };

    int best_match = 0;
    double best_zero_a = 0, best_zero_b = 0, best_zero_c = 0;
    double best_one_a = 0, best_one_b = 0, best_one_c = 0;

    for (double zero_a : values) {
        for (double one_a : values) {
            for (double zero_b : values) {
                for (double one_b : values) {
                    for (double zero_c : values) {
                        for (double one_c : values) {
                            int match = 0;
                            for (int L : {0, 1}) {
                                for (int C : {0, 1}) {
                                    for (int R : {0, 1}) {
                                        double l_val = L ? one_a : zero_a;
                                        double c_val = C ? one_b : zero_b;
                                        double r_val = R ? one_c : zero_c;
                                        double sum = l_val + c_val + r_val;
                                        int floor_val = (int)floor(sum);
                                        int mod2 = floor_val % 2;
                                        int expected = rule110[(L << 2) | (C << 1) | R];
                                        if (mod2 == expected) match++;
                                    }
                                }
                            }
                            if (match > best_match) {
                                best_match = match;
                                best_zero_a = zero_a;
                                best_one_a = one_a;
                                best_zero_b = zero_b;
                                best_one_b = one_b;
                                best_zero_c = zero_c;
                                best_one_c = one_c;
                            }
                        }
                    }
                }
            }
        }
    }

    cout << "  Best: " << best_match << "/8\n\n";

    if (best_match == 8) {
        cout << "  ✅ PERFECT 8/8!\n\n";
        cout << "  Encoding:\n";
        cout << "  L: 0→" << best_zero_a << ", 1→" << best_one_a << "\n";
        cout << "  C: 0→" << best_zero_b << ", 1→" << best_one_b << "\n";
        cout << "  R: 0→" << best_zero_c << ", 1→" << best_one_c << "\n\n";

        cout << "  L C R | Sum      | Floor | mod 2 | Expected\n";
        cout << "  ------|----------|-------|-------|----------\n";
        for (int L : {0, 1}) {
            for (int C : {0, 1}) {
                for (int R : {0, 1}) {
                    double l_val = L ? best_one_a : best_zero_a;
                    double c_val = C ? best_one_b : best_zero_b;
                    double r_val = R ? best_one_c : best_zero_c;
                    double sum = l_val + c_val + r_val;
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
    } else {
        cout << "  Walang 8/8 sa φ-based values.\n";
        cout << "  Best match: " << best_match << "/8\n";
        cout << "  L: 0→" << best_zero_a << ", 1→" << best_one_a << "\n";
        cout << "  C: 0→" << best_zero_b << ", 1→" << best_one_b << "\n";
        cout << "  R: 0→" << best_zero_c << ", 1→" << best_one_c << "\n";
    }

    return 0;
}
