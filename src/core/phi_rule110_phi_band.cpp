// ============================================
// φ-RULE 110 PHI BAND — φ-BASED THRESHOLD
//
// output = (rotated - φ⁻²) × (φ - rotated) > 0
// rotated = sum - φ⁻¹
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
    cout << "  φ-RULE 110 PHI BAND\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double PHI_INV2 = PHI_INV * PHI_INV;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // Tamang 8/8 weights
    const double W_L_ZERO = 0.0;
    const double W_L_ONE = pow(PHI, -3);
    const double W_C_ZERO = 0.0;
    const double W_C_ONE = pow(PHI, -2);
    const double W_R_ZERO = PHI / 2.0;
    const double W_R_ONE = PHI;

    // Band: [φ⁻², φ] sa rotated space
    const double BAND_LOW = PHI_INV2;   // φ⁻² = 0.382
    const double BAND_HIGH = PHI;        // φ = 1.618

    cout << "  Band: [" << BAND_LOW << ", " << BAND_HIGH << "]\n\n";

    cout << "  L C R | Sum      | Rotated  | In Band? | Output | Expected\n";
    cout << "  ------|----------|----------|----------|--------|----------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = (L ? W_L_ONE : W_L_ZERO) + 
                            (C ? W_C_ONE : W_C_ZERO) + 
                            (R ? W_R_ONE : W_R_ZERO);
                double rotated = sum - PHI_INV;
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool in_band = (rotated >= BAND_LOW && rotated <= BAND_HIGH);
                int output = in_band ? 1 : 0;
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(8) << fixed << setprecision(4) << rotated << " | "
                     << setw(8) << (in_band ? "YES" : "NO") << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // MAS MALAWAK NA SEARCH PARA SA PERFECT BAND
    // ============================================

    cout << "========================================\n";
    cout << "  SEARCH: PERFECT φ-BAND\n";
    cout << "========================================\n\n";

    double best_low = 0, best_high = 0;
    int best_match = 0;

    vector<double> phi_values = {
        0.0,
        pow(PHI, -6), pow(PHI, -5), pow(PHI, -4), pow(PHI, -3),
        pow(PHI, -2), pow(PHI, -1), 0.5, pow(PHI, -0.5),
        1.0, pow(PHI, 0.5), PHI, pow(PHI, 1.5), pow(PHI, 2),
        pow(PHI, 2.5), pow(PHI, 3)
    };

    for (double low : phi_values) {
        for (double high : phi_values) {
            if (high <= low) continue;
            int match = 0;
            for (int L : {0, 1}) {
                for (int C : {0, 1}) {
                    for (int R : {0, 1}) {
                        double sum = (L ? W_L_ONE : W_L_ZERO) + 
                                    (C ? W_C_ONE : W_C_ZERO) + 
                                    (R ? W_R_ONE : W_R_ZERO);
                        double rotated = sum - PHI_INV;
                        int expected = rule110[(L << 2) | (C << 1) | R];
                        int output = (rotated >= low && rotated <= high) ? 1 : 0;
                        if (output == expected) match++;
                    }
                }
            }
            if (match > best_match) {
                best_match = match;
                best_low = low;
                best_high = high;
            }
        }
    }

    cout << "  Best: " << best_match << "/8\n";
    cout << "  Low: " << best_low << " (φ^" << log(best_low) / log(PHI) << ")\n";
    cout << "  High: " << best_high << " (φ^" << log(best_high) / log(PHI) << ")\n\n";

    if (best_match == 8) {
        cout << "  ✅ PERFECT φ-BAND!\n";
        cout << "  Band: [" << best_low << ", " << best_high << "]\n";
    }

    return 0;
}
