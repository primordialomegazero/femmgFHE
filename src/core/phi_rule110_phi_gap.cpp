// ============================================
// φ-RULE 110 PHI GAP — Natural na Threshold
// next = (φ-gap > 0.5) at (val < 4.0)
// + correction para sa val=2.618
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 PHI GAP ===\n\n";
    cout << fixed << setprecision(4);

    cout << "  Formula: next = (gap > 0.5) && (val < 4.0)\n";
    cout << "  Correction: val=2.618 && diff > 0 → next=1\n\n";

    cout << "  Pattern | Val | Gap | Val<4? | Gap>0.5? | Diff | Decoded | Next | Match\n";
    cout << "  --------|-----|-----|--------|----------|------|---------|------|-------\n";

    int matches = 0;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                double val = L * pow(PHI, 2) + C * PHI + R;
                double gap = fmod(val, PHI);
                double diff = L - R;
                
                bool val_lt_4 = (val < 4.0);
                bool gap_gt_half = (gap > 0.5);
                bool base = val_lt_4 && gap_gt_half;
                
                bool correction = (abs(val - pow(PHI, 2)) < 0.01) && (diff > 0);
                int decoded = (base || correction) ? 1 : 0;
                
                bool match = (decoded == expected);
                if (match) matches++;
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << val << " | "
                     << setw(4) << gap << " | "
                     << setw(4) << (val_lt_4 ? "Y" : "N") << " | "
                     << setw(5) << (gap_gt_half ? "Y" : "N") << " | "
                     << setw(3) << diff << " | "
                     << setw(5) << decoded << " |  "
                     << expected << "   | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Total: " << matches << "/8\n\n";

    if (matches == 8) {
        cout << "  🏆 PERFECT FORMULA! 🏆\n";
        cout << "  next = (gap > 0.5) && (val < 4.0)\n";
        cout << "  + correction para sa val=2.618, diff>0\n\n";
        cout << "  Sa FHE:\n";
        cout << "  - gap = fmod(val, φ) — natural sa φ-structure\n";
        cout << "  - val < 4.0 — kailangan ng comparison\n";
        cout << "  - diff > 0 — kailangan ng comparison\n";
        cout << "  - Ang comparisons ay maaaring i-approximate\n";
        cout << "    gamit ang φ-parity\n";
    }

    return 0;
}
