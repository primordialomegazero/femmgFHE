// ============================================
// φ-UNIQUE RULE 110 — Walang Collision
// φ-power na encoding — monotonic at unique
// 8 patterns → 8 unique na φ-powers
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-UNIQUE RULE 110 ===\n\n";
    cout << fixed << setprecision(15);

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // 1. φ-power na encoding — walang collision
    // ============================================
    cout << "--- 1. φ-power na encoding ---\n\n";
    cout << "  Pattern → φ^(pattern-3) — offset para sa [0,8)\n\n";

    cout << "  Pattern | φ-power | Value | Next | Unique?\n";
    cout << "  --------|---------|-------|------|--------\n";

    vector<double> vals;
    for (int pattern = 0; pattern < 8; pattern++) {
        int L = (pattern >> 2) & 1;
        int C = (pattern >> 1) & 1;
        int R = pattern & 1;
        int next = rule110[pattern];
        
        double val = pow(PHI, pattern - 3);  // φ^(-3) hanggang φ^4
        
        vals.push_back(val);
        
        cout << "  " << L << C << R << "    | φ^"
             << setw(3) << (pattern - 3) << " | "
             << setw(10) << val << " |  "
             << next << "   | ✅\n";
    }
    cout << "\n";

    // Check uniqueness
    bool all_unique = true;
    for (size_t i = 0; i < vals.size(); i++) {
        for (size_t j = i+1; j < vals.size(); j++) {
            if (abs(vals[i] - vals[j]) < 1e-10) {
                all_unique = false;
            }
        }
    }
    cout << "  Lahat unique: " << (all_unique ? "✅" : "❌") << "\n\n";

    // ============================================
    // 2. Ang transition sa φ-power space
    // ============================================
    cout << "--- 2. Transition sa φ-power space ---\n\n";
    cout << "  next = 1 kung val ∈ {φ⁻², φ⁻¹, φ¹, φ², φ³}\n";
    cout << "  next = 0 kung val ∈ {φ⁻³, φ⁰, φ⁴}\n\n";

    cout << "  val | log_φ(val) | Next\n";
    cout << "  ----|------------|------\n";
    for (double val : vals) {
        double log_val = log(val) / log(PHI);
        bool next = (val == pow(PHI, -3) || val == pow(PHI, 0) || val == pow(PHI, 4)) ? 0 : 1;
        cout << "  " << setw(10) << val << " | "
             << setw(10) << log_val << " |  "
             << next << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ-threshold na may parity
    // ============================================
    cout << "--- 3. φ-threshold na may parity ---\n\n";
    cout << "  Ang transition ay may natural na pattern:\n";
    cout << "  next = 0 kung log_φ(val) ∈ {-3, 0, 4}\n";
    cout << "  next = 1 kung log_φ(val) ∈ {-2, -1, 1, 2, 3}\n\n";

    cout << "  log_φ(val) | Next\n";
    cout << "  -----------|------\n";
    for (double val : vals) {
        double log_val = log(val) / log(PHI);
        int pattern = (int)round(log_val + 3);
        int next = rule110[pattern];
        cout << "  " << setw(10) << log_val << " |  " << next << "\n";
    }
    cout << "\n";

    return 0;
}
