// ============================================
// φ-RULE 110 FORMULA — Hanapin ang Direct Formula
// May φ-based ba na transition formula?
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-RULE 110 FORMULA ===\n\n";
    cout << fixed << setprecision(12);

    // Rule 110 truth table
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // 1. Ang pattern at φ-power representation
    // ============================================
    cout << "--- 1. Pattern at φ-power ---\n\n";
    cout << "  Pattern | Next | φ^pattern | φ^next\n";
    cout << "  --------|------|-----------|-------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double phi_pattern = pow(PHI, pattern);
                double phi_next = pow(PHI, next);
                
                cout << "  " << L << C << R << "    |  "
                     << next << "   | "
                     << setw(10) << phi_pattern << " | "
                     << setw(10) << phi_next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang pattern bilang linear combination
    // ============================================
    cout << "--- 2. Pattern bilang linear ---\n\n";
    cout << "  Ang pattern ay maaaring i-encode bilang:\n";
    cout << "  val = L×φ² + C×φ + R\n\n";
    
    cout << "  Pattern | val = Lφ²+Cφ+R | Next\n";
    cout << "  --------|----------------|------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double val = L * PHI * PHI + C * PHI + R;
                
                cout << "  " << L << C << R << "    | "
                     << setw(12) << val << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 3. Ang transition bilang φ-gap
    // ============================================
    cout << "--- 3. Transition bilang φ-gap ---\n\n";
    cout << "  next = 1 kung ang val ay nasa φ-gap\n";
    cout << "  next = 0 kung hindi\n\n";
    
    cout << "  val | Next | φ-gap?\n";
    cout << "  ----|------|-------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double val = L * PHI * PHI + C * PHI + R;
                
                // Ang φ-gap ay: val mod φ ≈ 0 o φ⁻¹
                double mod_phi = fmod(val, PHI);
                bool in_gap = (mod_phi > 0.4 && mod_phi < 0.8);
                
                cout << "  " << setw(8) << val << " |  "
                     << next << "   | "
                     << (in_gap ? "yes" : "no") << "\n";
            }
        }
    }
    cout << "\n";

    return 0;
}
