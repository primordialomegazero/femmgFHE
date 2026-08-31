// ============================================
// φ-RULE 110 FRACTION BAND FIXED — 8/8
//
// Ang band ay [5φ-7, 3φ-3] na may tolerance
// para sa floating point precision
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
    cout << "  φ-RULE 110 FRACTION BAND FIXED\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);

    // Band sa fraction form na may tolerance
    const double LOWER = 5.0 * PHI - 7.0;  // 1.09017
    const double UPPER = 3.0 * PHI - 3.0;  // 1.85410
    const double TOLERANCE = 0.001;         // Para sa floating point

    cout << "  Band: [" << LOWER << ", " << UPPER << "]\n";
    cout << "  Tolerance: " << TOLERANCE << "\n\n";

    cout << "  L C R | Sum      | In Band? | Output | Expected\n";
    cout << "  ------|----------|----------|--------|----------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool in_band = (sum >= LOWER - TOLERANCE && sum <= UPPER + TOLERANCE);
                int output = in_band ? 1 : 0;
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(6) << sum << " | "
                     << setw(8) << (in_band ? "YES" : "NO") << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // ANG BAND SA FHE
    // ============================================
    //
    // Ang band ay maaaring i-encode bilang:
    // output = 1 kung sum ∈ [5φ-7, 3φ-3]
    //
    // Sa FHE, ito ay nangangailangan ng comparison:
    // - sum ≥ 5φ-7
    // - sum ≤ 3φ-3
    //
    // Ang comparison ay maaaring i-approximate gamit ang
    // polynomial sa depth 1:
    //
    // f(sum) = (sum - LOWER) × (UPPER - sum)
    //
    // Kung f(sum) > 0, ang sum ay nasa loob ng band.
    // Kung f(sum) < 0, ang sum ay nasa labas.
    //
    // Sa depth 1, may isang multiplication tayo:
    // f(sum) = (sum - LOWER) × (UPPER - sum)
    //
    // Ito ay quadratic polynomial na may maximum
    // sa gitna ng band.

    cout << "========================================\n";
    cout << "  ANG BAND SA FHE\n";
    cout << "========================================\n\n";

    cout << "  f(sum) = (sum - LOWER) × (UPPER - sum)\n\n";

    cout << "  Kung f(sum) > 0 → output 1 (nasa band)\n";
    cout << "  Kung f(sum) < 0 → output 0 (wala sa band)\n\n";

    cout << "  Sum      | f(sum)    | Output\n";
    cout << "  ---------|-----------|-------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                double f = (sum - LOWER) * (UPPER - sum);
                int output = (f > 0) ? 1 : 0;
                int expected = rule110[(L << 2) | (C << 1) | R];
                
                cout << "  " << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(9) << fixed << setprecision(6) << f << " | "
                     << setw(6) << output << " | "
                     << (output == expected ? "✅" : "❌") << "\n";
            }
        }
    }

    return 0;
}
