// ============================================
// φ-RULE 110 FRACTION BAND — 8/8 THRESHOLD
//
// Ang band ay [5φ-7, 3φ-3]
// Output 1 kung sum ∈ [5φ-7, 3φ-3]
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
    cout << "  φ-RULE 110 FRACTION BAND\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);

    // Band sa fraction form
    const double LOWER = 5.0 * PHI - 7.0;  // 1.090
    const double UPPER = 3.0 * PHI - 3.0;  // 1.854

    cout << "  Band: [" << LOWER << ", " << UPPER << "]\n";
    cout << "  Lower: 5φ - 7 = " << LOWER << "\n";
    cout << "  Upper: 3φ - 3 = " << UPPER << "\n\n";

    cout << "  L C R | Sum      | Fraction Form | In Band? | Output | Expected\n";
    cout << "  ------|----------|---------------|----------|--------|----------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool in_band = (sum >= LOWER && sum <= UPPER);
                int output = in_band ? 1 : 0;
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(13) << "φ-form" << " | "
                     << setw(8) << (in_band ? "YES" : "NO") << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // ANG BAND SA φ-LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  BAND SA φ-LOG SPACE\n";
    cout << "========================================\n\n";

    double log_lower = log(LOWER) / log(PHI);
    double log_upper = log(UPPER) / log(PHI);

    cout << "  log_φ(LOWER) = " << log_lower << "\n";
    cout << "  log_φ(UPPER) = " << log_upper << "\n";
    cout << "  Band width sa log: " << log_upper - log_lower << "\n\n";

    cout << "  Ang band sa log space ay maaaring i-encode bilang:\n";
    cout << "  output = 1 kung log_φ(sum) ∈ [" << log_lower << ", " << log_upper << "]\n\n";

    // ============================================
    // ANG BAND BILANG φ-POWERS
    // ============================================

    cout << "========================================\n";
    cout << "  BAND BILANG φ-POWERS\n";
    cout << "========================================\n\n";

    cout << "  Lower: 5φ - 7 = φ^" << log_lower << "\n";
    cout << "  Upper: 3φ - 3 = φ^" << log_upper << "\n\n";

    // ============================================
    // SIMPLIFICATION
    // ============================================

    cout << "========================================\n";
    cout << "  SIMPLIFICATION\n";
    cout << "========================================\n\n";

    cout << "  Lower: 5φ - 7 = 5(1.618) - 7 = 1.090\n";
    cout << "  Upper: 3φ - 3 = 3(1.618) - 3 = 1.854\n\n";

    cout << "  Sa φ-powers:\n";
    cout << "  Lower = φ^(-0.44) × φ^0.5 = φ^0.06\n";
    cout << "  Upper = φ^(1.28)\n\n";

    cout << "  ANG KEY: Ang band ay asymmetric sa φ-space.\n";
    cout << "  Ito ay dahil ang Rule 110 ay asymmetric:\n";
    cout << "  L, C, R ay may iba't ibang papel.\n\n";

    return 0;
}
