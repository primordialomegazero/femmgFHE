// ============================================
// φ-RULE 110 TRANSITION — Formula Hunt
// Hanapin ang φ-based na transition formula
// next_exp = f(exp_L, exp_C, exp_R)
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 TRANSITION ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Exponent encoding para sa 8 patterns
    // ============================================
    // 1 → exponent 0 (φ⁰ = 1)
    // 0 → exponent -3 (φ⁻³ = 0.236)
    
    cout << "--- 1. Exponent encoding ---\n\n";
    cout << "  Pattern | L_exp | C_exp | R_exp | Sum | Next\n";
    cout << "  --------|-------|-------|-------|-----|------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double L_exp = L ? 0.0 : -3.0;
                double C_exp = C ? 0.0 : -3.0;
                double R_exp = R ? 0.0 : -3.0;
                
                double sum = L_exp + C_exp + R_exp;
                
                cout << "  " << L << C << R << "    | "
                     << setw(5) << L_exp << " | "
                     << setw(5) << C_exp << " | "
                     << setw(5) << R_exp << " | "
                     << setw(4) << sum << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang sum ng exponents bilang threshold
    // ============================================
    cout << "--- 2. Sum bilang threshold ---\n\n";
    cout << "  Sum | Next | Threshold\n";
    cout << "  ----|------|----------\n";

    vector<double> sums;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double sum = (L ? 0.0 : -3.0) + (C ? 0.0 : -3.0) + (R ? 0.0 : -3.0);
                sums.push_back(sum);
                
                cout << "  " << setw(4) << sum << " |  "
                     << next << " | ";
                
                if (next == 0) cout << "below\n";
                else cout << "above\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 3. φ-based na formula
    // ============================================
    cout << "--- 3. φ-based na formula ---\n\n";
    cout << "  next = 1 kung φ^sum > threshold\n";
    cout << "  next = 0 kung φ^sum < threshold\n\n";

    cout << "  sum | φ^sum | Next\n";
    cout << "  ----|-------|------\n";

    for (double sum : sums) {
        double phi_sum = pow(PHI, sum);
        int pattern_idx = 0;
        for (int L = 0; L <= 1; L++) {
            for (int C = 0; C <= 1; C++) {
                for (int R = 0; R <= 1; R++) {
                    double s = (L ? 0.0 : -3.0) + (C ? 0.0 : -3.0) + (R ? 0.0 : -3.0);
                    if (abs(s - sum) < 0.01) {
                        int pattern = (L << 2) | (C << 1) | R;
                        int next = rule110[pattern];
                        cout << "  " << setw(4) << sum << " | "
                             << setw(8) << phi_sum << " |  "
                             << next << "\n";
                        break;
                    }
                }
            }
        }
    }
    cout << "\n";

    return 0;
}
