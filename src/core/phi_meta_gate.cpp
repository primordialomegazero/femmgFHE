// ============================================
// φ-META GATE — Mas Malalim na φ-structure
// Hindi lang parity — ang buong φ-representation
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-META GATE ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. φ-representation ng bawat pattern
    // ============================================
    cout << "--- 1. φ-representation ---\n\n";
    cout << "  Pattern | φ-power repr | φ-gap | φ-mod | Next\n";
    cout << "  --------|--------------|-------|-------|------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                // φ-power representation
                double val = L * pow(PHI, 2) + C * PHI + R;
                double mod_phi = fmod(val, PHI);
                
                // φ-gap: val - φ^n
                double gap = val - pow(PHI, (int)round(log(val) / log(PHI)));
                
                cout << "  " << L << C << R << "    | "
                     << setw(10) << val << " | "
                     << setw(5) << gap << " | "
                     << setw(5) << mod_phi << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang meta pattern
    // ============================================
    cout << "--- 2. Meta pattern ---\n\n";
    cout << "  Ang next=1 patterns ay:\n";
    cout << "  001, 010, 100, 101, 110\n\n";
    
    cout << "  Ang next=0 patterns ay:\n";
    cout << "  000, 011, 111\n\n";

    cout << "  next=1: 5 patterns — may φ-density\n";
    cout << "  next=0: 3 patterns — may φ-symmetry\n\n";

    // ============================================
    // 3. Ang φ-density ng next=1
    // ============================================
    cout << "--- 3. φ-density ---\n\n";
    cout << "  next=1 patterns: 001, 010, 100, 101, 110\n";
    cout << "  Count: 5/8 = 0.625\n";
    cout << "  φ⁻¹ = 0.618\n\n";
    cout << "  Ang density ng next=1 ay ≈ φ⁻¹!\n\n";

    // ============================================
    // 4. Ang meta gate formula
    // ============================================
    cout << "--- 4. Meta gate formula ---\n\n";
    cout << "  next = 1 kung val mod φ < φ⁻¹\n";
    cout << "  next = 0 kung val mod φ ≥ φ⁻¹\n\n";

    cout << "  val | mod φ | < φ⁻¹? | Next | Match\n";
    cout << "  ----|-------|---------|------|-------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                double val = L * pow(PHI, 2) + C * PHI + R;
                double mod_phi = fmod(val, PHI);
                bool threshold = mod_phi < (PHI - 1.0);
                int decoded = threshold ? 1 : 0;
                
                cout << "  " << setw(8) << val << " | "
                     << setw(5) << mod_phi << " | "
                     << setw(5) << (threshold ? "yes" : "no") << " |  "
                     << expected << "   | "
                     << (decoded == expected ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 5. Ang φ-gap bilang gate
    // ============================================
    cout << "--- 5. φ-gap bilang gate ---\n\n";
    cout << "  Ang φ-gap (val - φ^n) ay may natural\n";
    cout << "  na threshold para sa transition\n\n";

    cout << "  Pattern | val | φ^round | gap | Next\n";
    cout << "  --------|-----|---------|-----|------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                double val = L * pow(PHI, 2) + C * PHI + R;
                int n = (int)round(log(val) / log(PHI));
                double phi_n = pow(PHI, n);
                double gap = val - phi_n;
                
                cout << "  " << L << C << R << "    | "
                     << setw(5) << val << " | "
                     << setw(5) << phi_n << " | "
                     << setw(6) << gap << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    return 0;
}
