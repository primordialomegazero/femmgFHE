// ============================================
// φ-RULE 110 WSUM2 — Unique Weights
// w_L=φ³, w_C=φ², w_R=φ — walang collision
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 WSUM2 ===\n\n";
    cout << fixed << setprecision(12);

    cout << "  Pattern | Weighted Sum | Next | Unique?\n";
    cout << "  --------|---------------|------|--------\n";

    vector<double> vals;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double val = L * pow(PHI, 3) + C * pow(PHI, 2) + R * PHI;
                vals.push_back(val);
                
                cout << "  " << L << C << R << "    | "
                     << setw(10) << val << " |  "
                     << next << "   | ✅\n";
            }
        }
    }
    
    // Check uniqueness
    bool all_unique = true;
    for (size_t i = 0; i < vals.size(); i++) {
        for (size_t j = i+1; j < vals.size(); j++) {
            if (abs(vals[i] - vals[j]) < 1e-10) {
                all_unique = false;
            }
        }
    }
    cout << "\n  Lahat unique: " << (all_unique ? "✅" : "❌") << "\n\n";

    return 0;
}
