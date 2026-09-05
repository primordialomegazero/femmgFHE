// ============================================
// φ-RULE 110 ALT REALITY — Alternating Parity
// Sum + parity = walang collision
// φ-based na two-state na dimension
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    const double EXP_ZERO = -5.0;
    const double EXP_ONE = -2.0;

    cout << "=== φ-RULE 110 ALT REALITY ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Sum + parity analysis
    // ============================================
    cout << "--- 1. Sum + Parity ---\n\n";
    cout << "  Pattern | Sum | Parity | Next\n";
    cout << "  --------|-----|--------|------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double sum = (L ? EXP_ONE : EXP_ZERO) +
                             (C ? EXP_ONE : EXP_ZERO) +
                             (R ? EXP_ONE : EXP_ZERO);
                
                int parity = (L + C + R) % 2;
                
                cout << "  " << L << C << R << "    | "
                     << setw(4) << sum << " | "
                     << setw(5) << parity << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang alternating reality na transition
    // ============================================
    cout << "--- 2. Alternating reality ---\n\n";
    cout << "  Ang sum -9 ay may dalawang cases:\n";
    cout << "    parity=1 (011) → next=0\n";
    cout << "    parity=1 (101) → next=1 ← CONFLICT\n";
    cout << "    parity=0 (110) → next=1\n\n";

    cout << "  Ang sum -12 ay may dalawang cases:\n";
    cout << "    parity=1 (001) → next=1\n";
    cout << "    parity=1 (010) → next=1\n";
    cout << "    parity=0 (100) → next=1\n\n";

    // ============================================
    // 3. Ang φ-mod na alternating reality
    // ============================================
    cout << "--- 3. φ-mod reality ---\n\n";
    cout << "  Kung gagamitin ang φ-mod ng sum:\n";
    cout << "  sum mod φ ay may natural na cycle\n\n";

    cout << "  Pattern | sum mod φ | Next\n";
    cout << "  --------|-----------|------\n";
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                
                double sum = (L ? EXP_ONE : EXP_ZERO) +
                             (C ? EXP_ONE : EXP_ZERO) +
                             (R ? EXP_ONE : EXP_ZERO);
                
                double mod_phi = fmod(sum, PHI);
                
                cout << "  " << L << C << R << "    | "
                     << setw(8) << mod_phi << " |  "
                     << next << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 4. Ang emergent na alternating na formula
    // ============================================
    cout << "--- 4. Emergent formula ---\n\n";
    cout << "  next = f(sum, parity)\n";
    cout << "  next = 0 kung sum ∈ {-15, -9(011), -6}\n";
    cout << "  next = 1 kung iba\n\n";

    cout << "  Ang -9 na may (011) vs (101,110):\n";
    cout << "  011: sum=-9, binary=011 → next=0\n";
    cout << "  101: sum=-9, binary=101 → next=1\n";
    cout << "  110: sum=-9, binary=110 → next=1\n\n";

    cout << "  Ang difference ay nasa PATTERN ng bits:\n";
    cout << "  011 = 0,1,1 (center at right ay 1)\n";
    cout << "  101 = 1,0,1 (left at right ay 1)\n";
    cout << "  110 = 1,1,0 (left at center ay 1)\n\n";

    return 0;
}
