// ============================================
// φ-RULE 110 DECODE — Tamang Threshold
// (sum, diff) na may tamang binary decode
// Positive modulo + diff-based na selection
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    const double EXP_ZERO = -5.0;
    const double EXP_ONE = -2.0;

    cout << "=== φ-RULE 110 DECODE ===\n\n";
    cout << fixed << setprecision(4);

    // ============================================
    // 1. Tamang positive modulo
    // ============================================
    cout << "--- 1. Positive modulo ---\n\n";
    cout << "  Pattern | Sum | +mod φ | Diff | Next\n";
    cout << "  --------|-----|--------|------|------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double sum = (L ? EXP_ONE : EXP_ZERO) +
                             (C ? EXP_ONE : EXP_ZERO) +
                             (R ? EXP_ONE : EXP_ZERO);
                
                double diff = (L ? EXP_ONE : EXP_ZERO) -
                              (R ? EXP_ONE : EXP_ZERO);
                
                double pos_mod = fmod(fmod(sum, PHI) + PHI, PHI);
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << sum << " | "
                     << setw(6) << pos_mod << " | "
                     << setw(4) << diff << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang tamang decode formula
    // ============================================
    cout << "--- 2. Decode formula ---\n\n";
    cout << "  next = 1 kung sum == -12 (lahat)\n";
    cout << "  next = 1 kung sum == -9 at diff >= 0\n";
    cout << "  next = 0 kung sum == -15 o sum == -6\n";
    cout << "  next = 0 kung sum == -9 at diff < 0\n\n";

    // ============================================
    // 3. Verify ang decode
    // ============================================
    cout << "--- 3. Verification ---\n\n";
    cout << "  Pattern | Decoded | Expected | Match\n";
    cout << "  --------|---------|----------|-------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                double sum = (L ? EXP_ONE : EXP_ZERO) +
                             (C ? EXP_ONE : EXP_ZERO) +
                             (R ? EXP_ONE : EXP_ZERO);
                
                double diff = (L ? EXP_ONE : EXP_ZERO) -
                              (R ? EXP_ONE : EXP_ZERO);
                
                int decoded;
                if (abs(sum + 12.0) < 0.01) decoded = 1;
                else if (abs(sum + 15.0) < 0.01) decoded = 0;
                else if (abs(sum + 6.0) < 0.01) decoded = 0;
                else if (abs(sum + 9.0) < 0.01) {
                    decoded = (diff >= 0) ? 1 : 0;
                }
                else decoded = 0;
                
                bool match = (decoded == expected);
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << decoded << " | "
                     << setw(4) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n";

    return 0;
}
