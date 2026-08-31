// ============================================
// φ-RULE 110 CENTER QUAD — φ-CENTERED QUADRATIC
//
// f(sum) = δ² - (sum - center)²
// δ = φ⁻², center = (LOWER+UPPER)/2
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
    cout << "  φ-RULE 110 CENTER QUAD\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);

    // Center at δ
    const double LOWER = 5.0 * PHI - 7.0;
    const double UPPER = 3.0 * PHI - 3.0;
    const double CENTER = (LOWER + UPPER) / 2.0;
    const double DELTA = (UPPER - LOWER) / 2.0;

    cout << "  Lower: " << LOWER << "\n";
    cout << "  Upper: " << UPPER << "\n";
    cout << "  Center: " << CENTER << "\n";
    cout << "  Delta: " << DELTA << " ≈ φ⁻² = " << pow(PHI, -2) << "\n\n";

    // ============================================
    // QUADRATIC BAND FUNCTION
    // ============================================

    cout << "========================================\n";
    cout << "  QUADRATIC BAND FUNCTION\n";
    cout << "========================================\n\n";

    cout << "  f(sum) = δ² - (sum - center)²\n\n";

    cout << "  Sum      | f(sum)    | Output | Expected\n";
    cout << "  ---------|-----------|--------|----------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                double f = DELTA * DELTA - (sum - CENTER) * (sum - CENTER);
                int output = (f > 0) ? 1 : 0;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(9) << fixed << setprecision(6) << f << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // ANG KEY: δ = φ⁻²
    // ============================================

    cout << "========================================\n";
    cout << "  ANG KEY: δ = φ⁻²\n";
    cout << "========================================\n\n";

    cout << "  δ = " << DELTA << "\n";
    cout << "  φ⁻² = " << pow(PHI, -2) << "\n";
    cout << "  Difference: " << abs(DELTA - pow(PHI, -2)) << "\n\n";

    cout << "  Center = " << CENTER << "\n";
    cout << "  φ⁰·⁸ = " << pow(PHI, 0.8) << "\n";
    cout << "  φ/2 + φ⁻¹/2 = " << (PHI / 2.0 + pow(PHI, -1) / 2.0) << "\n\n";

    // ============================================
    // PURE φ-BASED CENTER AT DELTA
    // ============================================

    cout << "========================================\n";
    cout << "  PURE φ-BASED CENTER AT DELTA\n";
    cout << "========================================\n\n";

    // Subukan: δ = φ⁻², center = φ - φ⁻²
    double delta_phi = pow(PHI, -2);
    double center_phi = PHI - delta_phi;

    cout << "  δ = φ⁻² = " << delta_phi << "\n";
    cout << "  center = φ - φ⁻² = " << center_phi << "\n\n";

    cout << "  f(sum) = φ⁻⁴ - (sum - (φ - φ⁻²))²\n\n";

    cout << "  Sum      | f(sum)    | Output | Expected\n";
    cout << "  ---------|-----------|--------|----------\n";

    int match2 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                double f = delta_phi * delta_phi - (sum - center_phi) * (sum - center_phi);
                int output = (f > 0) ? 1 : 0;
                int expected = rule110[(L << 2) | (C << 1) | R];
                if (output == expected) match2++;
                
                cout << "  " << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(9) << fixed << setprecision(6) << f << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (output == expected ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match with pure φ: " << match2 << "/8\n";

    return 0;
}
