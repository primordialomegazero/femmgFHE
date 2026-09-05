// ============================================
// φ-META FIB WSUM — Irrational Weights
// val = L×φ⁴ + C×φ² + R×φ⁰
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

    cout << "=== φ-META FIB WSUM ===\n\n";
    cout << fixed << setprecision(15);

    // Meta-Fibonacci weights: φ⁴, φ², φ⁰
    // Hindi sila magka-collide kasi irrational
    double w_L = pow(PHI, 4);  // 6.854
    double w_C = pow(PHI, 2);  // 2.618
    double w_R = pow(PHI, 0);  // 1.0

    cout << "  Weights: L=" << w_L << ", C=" << w_C << ", R=" << w_R << "\n\n";
    cout << "  Pattern | Weighted Sum | Next | Unique?\n";
    cout << "  --------|---------------|------|--------\n";

    vector<double> vals;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double val = L * w_L + C * w_C + R * w_R;
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

    // Ang transition ay maaaring i-encode bilang threshold sa φ-space
    cout << "  val sorted:\n";
    sort(vals.begin(), vals.end());
    for (size_t i = 0; i < vals.size(); i++) {
        int pattern_idx = i;
        int next = rule110[pattern_idx];
        cout << "    " << setw(14) << vals[i] << " → next=" << next << "\n";
    }
    cout << "\n";

    return 0;
}
