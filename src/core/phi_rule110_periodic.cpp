// ============================================
// φ-RULE 110 PERIODIC — HANAPIN ANG PERIODICITY
//
// Ang weights ay dapat may φ-periodicity:
// L → C → R ay dapat φ-related
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
    cout << "  φ-RULE 110 PERIODIC\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double LN_PHI = log(PHI);

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // ANG NATAGPUANG PERFECT ENCODING
    // ============================================

    cout << "========================================\n";
    cout << "  PERFECT ENCODING ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  Encoding:\n";
    cout << "  L: 0→0.000, 1→0.236 (φ⁻³)\n";
    cout << "  C: 0→0.000, 1→0.382 (φ⁻²)\n";
    cout << "  R: 0→0.809, 1→1.618 (φ/2 at φ)\n\n";

    cout << "  Sa φ-powers:\n";
    cout << "  L: 0→-∞, 1→-3\n";
    cout << "  C: 0→-∞, 1→-2\n";
    cout << "  R: 0→-0.44, 1→1\n\n";

    cout << "  Periodicity:\n";
    cout << "  L→C: -3 → -2 (×φ¹)\n";
    cout << "  C→R: -2 → 1 (×φ³)\n\n";

    // ============================================
    // SUBUKAN ANG PERIODIC WEIGHTS
    // ============================================

    cout << "========================================\n";
    cout << "  PERIODIC WEIGHTS SEARCH\n";
    cout << "========================================\n\n";

    cout << "  Subukan: L→φ^a, C→φ^b, R→φ^c\n";
    cout << "  kung saan a, b, c ay integers\n\n";

    int best_match = 0;
    double best_a = 0, best_b = 0, best_c = 0;

    for (int a = -5; a <= 5; a++) {
        for (int b = -5; b <= 5; b++) {
            for (int c = -5; c <= 5; c++) {
                int match = 0;
                for (int L : {0, 1}) {
                    for (int C : {0, 1}) {
                        for (int R : {0, 1}) {
                            double l_val = L ? pow(PHI, a) : 0.0;
                            double c_val = C ? pow(PHI, b) : 0.0;
                            double r_val = R ? pow(PHI, c) : 0.0;
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
                    best_a = a;
                    best_b = b;
                    best_c = c;
                }
            }
        }
    }

    cout << "  Best: " << best_match << "/8\n";
    cout << "  L→φ^" << best_a << " = " << pow(PHI, best_a) << "\n";
    cout << "  C→φ^" << best_b << " = " << pow(PHI, best_b) << "\n";
    cout << "  R→φ^" << best_c << " = " << pow(PHI, best_c) << "\n\n";

    // ============================================
    // SUBUKAN ANG φ-OFFSET PERIODICITY
    // ============================================

    cout << "========================================\n";
    cout << "  φ-OFFSET PERIODICITY\n";
    cout << "========================================\n\n";

    cout << "  Subukan: 0→φ^a, 1→φ^(a+k) na may period k\n\n";

    best_match = 0;
    double best_zero_exp = 0, best_period = 0;
    double best_ol = 0, best_oc = 0, best_or = 0;

    for (double zero_exp = -5; zero_exp <= 5; zero_exp += 0.5) {
        for (double period = 0.5; period <= 5; period += 0.5) {
            for (double offset_l = -2; offset_l <= 2; offset_l += 0.5) {
                for (double offset_c = -2; offset_c <= 2; offset_c += 0.5) {
                    for (double offset_r = -2; offset_r <= 2; offset_r += 0.5) {
                        int match = 0;
                        for (int L : {0, 1}) {
                            for (int C : {0, 1}) {
                                for (int R : {0, 1}) {
                                    double l_val = L ? pow(PHI, zero_exp + period + offset_l) : pow(PHI, zero_exp + offset_l);
                                    double c_val = C ? pow(PHI, zero_exp + period + offset_c) : pow(PHI, zero_exp + offset_c);
                                    double r_val = R ? pow(PHI, zero_exp + period + offset_r) : pow(PHI, zero_exp + offset_r);
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
                            best_zero_exp = zero_exp;
                            best_period = period;
                            best_ol = offset_l;
                            best_oc = offset_c;
                            best_or = offset_r;
                        }
                    }
                }
            }
        }
    }

    cout << "  Best: " << best_match << "/8\n";
    cout << "  Zero exponent: " << best_zero_exp << "\n";
    cout << "  Period: " << best_period << "\n";
    cout << "  Offsets: L=" << best_ol << ", C=" << best_oc << ", R=" << best_or << "\n\n";

    if (best_match == 8) {
        cout << "  ✅ PERFECT PERIODIC ENCODING!\n\n";
        cout << "  0 → φ^" << best_zero_exp << " = " << pow(PHI, best_zero_exp) << "\n";
        cout << "  1 → φ^" << best_zero_exp + best_period << " = " << pow(PHI, best_zero_exp + best_period) << "\n\n";
        
        cout << "  L C R | Sum      | Floor | mod 2 | Expected\n";
        cout << "  ------|----------|-------|-------|----------\n";
        for (int L : {0, 1}) {
            for (int C : {0, 1}) {
                for (int R : {0, 1}) {
                    double l_val = L ? pow(PHI, best_zero_exp + best_period + best_ol) : pow(PHI, best_zero_exp + best_ol);
                    double c_val = C ? pow(PHI, best_zero_exp + best_period + best_oc) : pow(PHI, best_zero_exp + best_oc);
                    double r_val = R ? pow(PHI, best_zero_exp + best_period + best_or) : pow(PHI, best_zero_exp + best_or);
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
    }

    // ============================================
    // ANG KEY: FRACTIONAL φ-POWERS
    // ============================================

    cout << "\n========================================\n";
    cout << "  FRACTIONAL φ-POWERS\n";
    cout << "========================================\n\n";

    cout << "  Ang 0.809 = φ/2 ay may φ-log:\n";
    cout << "  log_φ(0.809) = " << log(0.809) / LN_PHI << "\n\n";

    cout << "  φ^(1/2) = " << sqrt(PHI) << "\n";
    cout << "  φ^(3/2) = " << pow(PHI, 1.5) << "\n";
    cout << "  φ^(1/3) = " << pow(PHI, 1.0/3.0) << "\n";
    cout << "  φ^(2/3) = " << pow(PHI, 2.0/3.0) << "\n\n";

    cout << "  Ang 0.809 = φ/2 ay hindi eksaktong φ-power\n";
    cout << "  Pero: φ/2 = φ × 1/2 = φ × φ^(-log_φ(2))\n";
    cout << "  log_φ(2) = " << log(2.0) / LN_PHI << "\n";
    cout << "  Kaya: φ/2 = φ^(1 - log_φ(2)) = φ^" << 1.0 - log(2.0) / LN_PHI << "\n\n";

    // ============================================
    // ANG PERIODICITY SA TRANSITION TABLE
    // ============================================

    cout << "========================================\n";
    cout << "  PERIODICITY SA TRANSITION TABLE\n";
    cout << "========================================\n\n";

    cout << "  Ang 8 sums (sorted):\n\n";

    vector<pair<double, string>> sums;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = L ? 0.236 : 0.0;
                double c_val = C ? 0.382 : 0.0;
                double r_val = R ? 1.618 : 0.809;
                double sum = l_val + c_val + r_val;
                string pattern = to_string(L) + to_string(C) + to_string(R);
                sums.push_back({sum, pattern});
            }
        }
    }

    sort(sums.begin(), sums.end());

    for (auto& [sum, pattern] : sums) {
        int floor_val = (int)floor(sum);
        cout << "  " << pattern << " → " << setw(8) << fixed << setprecision(4) << sum
             << " → floor " << floor_val << " → mod 2 = " << floor_val % 2 << "\n";
    }

    cout << "\n  Ang sums ay may φ-harmonic structure:\n";
    cout << "  0.809, 1.045, 1.191, 1.427, 1.618, 1.854, 2.000, 2.236\n\n";

    cout << "  Differences:\n";
    cout << "  1.045 - 0.809 = 0.236 = φ⁻³\n";
    cout << "  1.191 - 1.045 = 0.146 = φ⁻⁴\n";
    cout << "  1.427 - 1.191 = 0.236 = φ⁻³\n";
    cout << "  1.618 - 1.427 = 0.191 ≈ φ⁻³×φ/2\n";
    cout << "  1.854 - 1.618 = 0.236 = φ⁻³\n";
    cout << "  2.000 - 1.854 = 0.146 = φ⁻⁴\n";
    cout << "  2.236 - 2.000 = 0.236 = φ⁻³\n\n";

    cout << "  PERIODICITY: φ⁻³, φ⁻⁴, φ⁻³ | φ⁻³×φ/2, φ⁻³, φ⁻⁴ | φ⁻³\n";
    cout << "  Period-3 structure!\n\n";

    return 0;
}
