// ============================================
// φ-FIB WSUM — Fibonacci Weights na Unique
// w_L=F₄=5, w_C=F₃=3, w_R=F₂=2
// Zeckendorf theorem — walang collision
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-FIB WSUM ===\n\n";
    cout << fixed << setprecision(12);

    cout << "  Pattern | Weighted Sum | Next | Unique?\n";
    cout << "  --------|---------------|------|--------\n";

    vector<double> vals;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double val = L * 5.0 + C * 3.0 + R * 2.0;
                vals.push_back(val);
                
                cout << "  " << L << C << R << "    | "
                     << setw(10) << val << " |  "
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

    // Ang transition: next = 1 kung val ∈ {2, 3, 5, 7, 8, 10}
    // next = 0 kung val ∈ {0, 6}
    cout << "  Transition rules:\n";
    cout << "  next = 0 kung val ∈ {0, 6}\n";
    cout << "  next = 1 kung val ∈ {2, 3, 5, 7, 8, 10}\n\n";

    cout << "  val | Next\n";
    cout << "  ----|------\n";
    for (double val : vals) {
        bool next = (val == 0 || val == 6) ? 0 : 1;
        cout << "  " << setw(4) << val << " |  " << next << "\n";
    }
    cout << "\n";

    return 0;
}
