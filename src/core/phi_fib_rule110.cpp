// ============================================
// φ-FIB RULE 110 — Fibonacci-Weighted Encoding
// val = L×F₁ + C×F₂ + R×F₃
// Unique na representation — walang collision
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-FIB RULE 110 ===\n\n";
    cout << fixed << setprecision(15);

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // Fibonacci weights
    vector<long long> fib = {0, 1, 1, 2, 3, 5, 8};

    // ============================================
    // 1. Fibonacci-weighted encoding
    // ============================================
    cout << "--- 1. Fibonacci-weighted encoding ---\n\n";
    cout << "  val = L×F₁ + C×F₂ + R×F₃\n\n";

    cout << "  Pattern | val | Next | Unique?\n";
    cout << "  --------|-----|------|--------\n";

    vector<double> vals;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double val = L * fib[3] + C * fib[4] + R * fib[5];
                vals.push_back(val);
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << val << " |  "
                     << next << "   |\n";
            }
        }
    }
    cout << "\n";

    // Check uniqueness
    bool all_unique = true;
    for (size_t i = 0; i < vals.size(); i++) {
        for (size_t j = i+1; j < vals.size(); j++) {
            if (abs(vals[i] - vals[j]) < 1e-10) {
                all_unique = false;
                cout << "  COLLISION: pattern " << i << " at " << j << "\n";
            }
        }
    }
    cout << "  Lahat unique: " << (all_unique ? "✅" : "❌") << "\n\n";

    // ============================================
    // 2. Ang transition formula
    // ============================================
    cout << "--- 2. Transition formula ---\n\n";
    cout << "  next = 1 kung val ∈ {2, 3, 5, 6, 7, 8}\n";
    cout << "  next = 0 kung val ∈ {0, 4}\n\n";

    cout << "  val | Next\n";
    cout << "  ----|------\n";
    for (double val : vals) {
        bool next = (val == 0 || val == 4) ? 0 : 1;
        cout << "  " << setw(3) << val << " |  " << next << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ-based na threshold
    // ============================================
    cout << "--- 3. φ-based na threshold ---\n\n";
    cout << "  Ang transition ay may natural na φ-threshold\n";
    cout << "  next = 1 kung val > φ (maliban sa val=4)\n\n";

    for (double val : vals) {
        bool next = (val == 0 || val == 4) ? 0 : 1;
        bool threshold = val > PHI;
        
        cout << "  val=" << setw(3) << val 
             << ": next=" << next 
             << ", φ-threshold=" << (threshold ? "above" : "below") << "\n";
    }
    cout << "\n";

    return 0;
}
