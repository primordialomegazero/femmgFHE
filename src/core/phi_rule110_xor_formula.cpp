// ============================================
// φ-RULE 110 XOR FORMULA — Emergent na XOR
// next = C^R ⊕ (~L&C) ⊕ (~C & (L==R))
// Hanapin ang φ-based na XOR
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 XOR FORMULA ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Test ang bagong formula
    // ============================================
    cout << "--- 1. New formula ---\n\n";
    cout << "  next = C^R ⊕ (~L&C) ⊕ (~C & (L==R))\n\n";

    cout << "  Pattern | C^R | ~L&C | ~C&(L==R) | next | Formula | Match\n";
    cout << "  --------|-----|------|-----------|------|---------|-------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                int xor_cr = C ^ R;
                int and_nl_c = (1 - L) & C;
                int and_nc_eq = (1 - C) & (L == R);
                int formula = xor_cr ^ and_nl_c ^ and_nc_eq;
                
                bool match = (formula == expected);
                
                cout << "  " << L << C << R << "    | "
                     << setw(2) << xor_cr << " | "
                     << setw(3) << and_nl_c << " | "
                     << setw(7) << and_nc_eq << " |  "
                     << expected << "   | "
                     << setw(4) << formula << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang XOR bilang φ-parity
    // ============================================
    cout << "--- 2. XOR bilang φ-parity ---\n\n";
    cout << "  Ang XOR ay parity ng sum:\n";
    cout << "  a XOR b = (a + b) mod 2\n\n";
    cout << "  Sa φ-space, ang parity ay:\n";
    cout << "  φ^(a+b) mod φ — natural na XOR\n\n";

    cout << "  a | b | a^b | (a+b)%2 | φ-mod\n";
    cout << "  --|---|-----|---------|-------\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            int xor_ab = a ^ b;
            int mod2 = (a + b) % 2;
            double phi_mod = fmod(pow((1.0 + sqrt(5.0)) / 2.0, a + b), (1.0 + sqrt(5.0)) / 2.0);
            
            cout << "  " << a << " | " << b << " | "
                 << setw(2) << xor_ab << " | "
                 << setw(4) << mod2 << " | "
                 << setw(6) << phi_mod << "\n";
        }
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ-based na XOR para sa Rule 110
    // ============================================
    cout << "--- 3. φ-based na XOR ---\n\n";
    cout << "  Ang formula ay may 3 XOR components:\n";
    cout << "  1. C^R — center XOR right\n";
    cout << "  2. ~L&C — NOT left AND center\n";
    cout << "  3. ~C&(L==R) — NOT center AND (left equals right)\n\n";
    cout << "  Lahat ay maaaring i-encode bilang φ-parity\n\n";

    return 0;
}
