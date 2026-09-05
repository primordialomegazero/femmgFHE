// ============================================
// φ-RULE 110 PARITY+R — Natural na Formula
// next = parity(val) XOR (R == 0)
// Walang explicit na comparison
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 PARITY+R ===\n\n";
    cout << fixed << setprecision(12);

    cout << "--- Formula: next = parity(val) XOR (R==0) ---\n\n";
    cout << "  Pattern | val | parity | R==0 | formula | Next | Match\n";
    cout << "  --------|-----|--------|-------|---------|------|-------\n";

    int matches = 0;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                double val = L * pow(PHI, 2) + C * PHI + R;
                int parity = (int)floor(val) % 2;
                int r_zero = (R == 0) ? 1 : 0;
                int formula = parity ^ r_zero;
                
                bool match = (formula == expected);
                if (match) matches++;
                
                cout << "  " << L << C << R << "    | "
                     << setw(5) << val << " | "
                     << setw(4) << parity << " | "
                     << setw(4) << r_zero << " | "
                     << setw(5) << formula << " |  "
                     << expected << "   | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Total: " << matches << "/8\n\n";

    if (matches == 8) {
        cout << "  🏆 PERFECT FORMULA! 🏆\n";
        cout << "  next = parity(L×φ² + C×φ + R) XOR (R==0)\n";
        cout << "  Walang explicit na comparison!\n";
        cout << "  Ang parity ay natural sa φ-structure\n";
        cout << "  Ang R==0 ay natural sa φ-parity\n";
    }

    return 0;
}
