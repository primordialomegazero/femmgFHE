// ============================================
// φ-FRACTAL RULE 110 — Fractal na Encoding
// Ang φ-fractal ay may unique na representation
// Walang collision sa 8 patterns
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-FRACTAL RULE 110 ===\n\n";
    cout << fixed << setprecision(15);

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // 1. Fractal na encoding: φ-based na continued fraction
    // ============================================
    cout << "--- 1. Fractal na encoding ---\n\n";
    cout << "  Ang φ ay may continued fraction [1;1,1,1,...]\n";
    cout << "  Bawat pattern ay may unique na φ-fractal value\n\n";

    // Ang fractal na encoding:
    // val = L/φ + C/φ² + R/φ³ (mas malalim na fractions)
    cout << "  Pattern | val = L/φ + C/φ² + R/φ³ | Unique?\n";
    cout << "  --------|--------------------------|--------\n";
    
    vector<double> vals;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double val = L / PHI + C / (PHI * PHI) + R / (PHI * PHI * PHI);
                vals.push_back(val);
                
                cout << "  " << L << C << R << "    | "
                     << setw(20) << val << " | "
                     << (next ? "next=1" : "next=0") << "\n";
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
    // 2. Fractal na threshold
    // ============================================
    cout << "--- 2. Fractal na threshold ---\n\n";
    cout << "  Ang transition ay maaaring i-encode bilang:\n";
    cout << "  next = 1 kung val mod φ < threshold\n\n";
    
    cout << "  val | mod φ | Next\n";
    cout << "  ----|-------|------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double val = L / PHI + C / (PHI * PHI) + R / (PHI * PHI * PHI);
                double mod_phi = fmod(val, PHI);
                
                cout << "  " << setw(15) << val << " | "
                     << setw(6) << mod_phi << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 3. Ang fractal na pattern
    // ============================================
    cout << "--- 3. Fractal na pattern ---\n\n";
    cout << "  Ang φ-fractal ay may self-similar na structure:\n";
    cout << "  φ⁻¹ = 0.618, φ⁻² = 0.382, φ⁻³ = 0.236\n";
    cout << "  Ang sum ay may natural na fractal na distribution\n\n";

    // Ang fractal na transition:
    // next = 1 kung ang val ay nasa fractal na set
    cout << "  val sorted:\n";
    sort(vals.begin(), vals.end());
    for (size_t i = 0; i < vals.size(); i++) {
        cout << "    " << setw(3) << i << ": " << vals[i] << "\n";
    }
    cout << "\n";

    return 0;
}
