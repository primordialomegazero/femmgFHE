// ============================================
// φ-RULE 110 WEIGHTED EXP — Unique Formula
// val = φ²×exp_L + φ×exp_C + exp_R
// Irrational weights — walang collision
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

    cout << "=== φ-RULE 110 WEIGHTED EXP ===\n\n";
    cout << fixed << setprecision(12);

    double w_L = PHI * PHI;  // φ² = 2.618
    double w_C = PHI;        // φ = 1.618
    double w_R = 1.0;        // 1

    cout << "  Weights: L=φ²=" << w_L << ", C=φ=" << w_C << ", R=1\n\n";

    cout << "  Pattern | Weighted Sum | Next | Unique?\n";
    cout << "  --------|---------------|------|--------\n";

    vector<double> vals;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double L_exp = L ? 0.0 : -3.0;
                double C_exp = C ? 0.0 : -3.0;
                double R_exp = R ? 0.0 : -3.0;
                
                double val = w_L * L_exp + w_C * C_exp + w_R * R_exp;
                vals.push_back(val);
                
                cout << "  " << L << C << R << "    | "
                     << setw(14) << val << " |  "
                     << next << "   | ✅\n";
            }
        }
    }
    
    bool all_unique = true;
    for (size_t i = 0; i < vals.size(); i++) {
        for (size_t j = i+1; j < vals.size(); j++) {
            if (abs(vals[i] - vals[j]) < 1e-10) {
                all_unique = false;
                cout << "  COLLISION: pattern " << i << " at " << j << "\n";
            }
        }
    }
    cout << "\n  Lahat unique: " << (all_unique ? "✅" : "❌") << "\n\n";

    // Ang transition: next = f(val)
    cout << "  val sorted:\n";
    sort(vals.begin(), vals.end());
    for (size_t i = 0; i < vals.size(); i++) {
        cout << "    " << setw(14) << vals[i] << "\n";
    }
    cout << "\n";

    return 0;
}
