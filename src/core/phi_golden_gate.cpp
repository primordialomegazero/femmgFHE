// ============================================
// φ-GOLDEN GATE — Natural na φ-gate
// GR(a,b,c) = φ^(a+b+c) mod φ
// Natural na parity-based na gate
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-GOLDEN GATE ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Ang Golden Gate definition
    // ============================================
    cout << "--- 1. Golden Gate ---\n\n";
    cout << "  GR(a,b,c) = φ^(a+b+c) mod φ\n";
    cout << "  Natural na φ-parity gate\n\n";

    cout << "  Pattern | Sum | φ^sum | φ^sum mod φ | Parity | Next\n";
    cout << "  --------|-----|-------|-------------|--------|------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                int sum = L + C + R;
                double phi_sum = pow(PHI, sum);
                double mod_phi = fmod(phi_sum, PHI);
                int parity = (mod_phi > 0.5) ? 0 : 1;
                
                cout << "  " << L << C << R << "    | "
                     << setw(3) << sum << " | "
                     << setw(5) << phi_sum << " | "
                     << setw(8) << mod_phi << " | "
                     << setw(5) << parity << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang Golden Gate para sa Rule 110
    // ============================================
    cout << "--- 2. Golden Gate para sa Rule 110 ---\n\n";
    cout << "  Ang GR gate ay nagbibigay ng parity\n";
    cout << "  na may natural na φ-structure\n\n";

    cout << "  Parity | Next | Match?\n";
    cout << "  -------|------|-------\n";
    
    int matches = 0;
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                int sum = L + C + R;
                double mod_phi = fmod(pow(PHI, sum), PHI);
                int parity = (mod_phi > 0.5) ? 0 : 1;
                
                bool match = (parity == expected);
                if (match) matches++;
                
                cout << "  " << setw(4) << parity << " |  "
                     << expected << "   | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Total: " << matches << "/8\n\n";

    // ============================================
    // 3. Ang composite Golden Gate
    // ============================================
    cout << "--- 3. Composite Golden Gate ---\n\n";
    cout << "  Kailangan ng composite gate:\n";
    cout << "  GR2(a,b,c) = GR(a,b,c) XOR correction(a,b,c)\n\n";
    
    cout << "  Ang correction ay para sa conflicts:\n";
    cout << "  011: correction = 1\n";
    cout << "  100: correction = 1\n";
    cout << "  Iba: correction = 0\n\n";

    cout << "  Pattern | GR parity | Correction | Next\n";
    cout << "  --------|-----------|------------|------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                int sum = L + C + R;
                double mod_phi = fmod(pow(PHI, sum), PHI);
                int parity = (mod_phi > 0.5) ? 0 : 1;
                
                bool is_conflict = (pattern == 3 || pattern == 4);
                int correction = is_conflict ? 1 : 0;
                int corrected = parity ^ correction;
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << parity << " | "
                     << setw(5) << correction << " |  "
                     << corrected << "\n";
            }
        }
    }
    cout << "\n";

    return 0;
}
