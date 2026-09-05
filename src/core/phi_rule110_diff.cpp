// ============================================
// φ-RULE 110 DIFF — Asymmetric Difference
// next = f(sum, left-right diff)
// Walang collision — natural na φ-structure
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

    cout << "=== φ-RULE 110 DIFF ===\n\n";
    cout << fixed << setprecision(12);

    cout << "  Pattern | Sum | Diff(L-R) | Next\n";
    cout << "  --------|-----|-----------|------\n";

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
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << sum << " | "
                     << setw(6) << diff << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // Ang key insight: sum at diff ay may unique na combination
    cout << "  Ang (sum, diff) ay unique para sa bawat pattern\n\n";

    // Check uniqueness
    vector<pair<double,double>> vals;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                double sum = (L ? EXP_ONE : EXP_ZERO) +
                             (C ? EXP_ONE : EXP_ZERO) +
                             (R ? EXP_ONE : EXP_ZERO);
                double diff = (L ? EXP_ONE : EXP_ZERO) - 
                              (R ? EXP_ONE : EXP_ZERO);
                vals.push_back({sum, diff});
            }
        }
    }

    bool all_unique = true;
    for (size_t i = 0; i < vals.size(); i++) {
        for (size_t j = i+1; j < vals.size(); j++) {
            if (abs(vals[i].first - vals[j].first) < 0.01 &&
                abs(vals[i].second - vals[j].second) < 0.01) {
                all_unique = false;
            }
        }
    }
    cout << "  Lahat unique: " << (all_unique ? "✅" : "❌") << "\n\n";

    return 0;
}
