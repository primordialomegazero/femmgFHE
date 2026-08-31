// ============================================
// φ-RULE 110 MOD2 — BEATTY MODULO 2
//
// XOR(A,B) = A + B (mod 2)
// Sa φ-log space: gumamit ng φ-based modulo
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 MOD2\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // XOR VIA MODULO 2
    // ============================================

    cout << "========================================\n";
    cout << "  XOR VIA MODULO 2\n";
    cout << "========================================\n\n";

    cout << "  A B | A+B | (A+B) mod 2 | XOR\n";
    cout << "  ----|-----|-------------|----\n";

    for (int a : {0, 1}) {
        for (int b : {0, 1}) {
            int sum = a + b;
            int mod2 = sum % 2;
            int xor_val = a ^ b;
            
            cout << "  " << a << " " << b << " | "
                 << setw(3) << sum << " | "
                 << setw(11) << mod2 << " | "
                 << setw(3) << xor_val << " | "
                 << (mod2 == xor_val ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n  XOR = (A + B) mod 2  ✅\n\n";

    // ============================================
    // MODULO 2 SA φ-LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  MODULO 2 SA φ-LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  Sa φ-log space, ang modulo 2 ay maaaring\n";
    cout << "  i-express gamit ang φ-periodicity:\n\n";

    cout << "  φ² = φ + 1\n";
    cout << "  φ² - φ - 1 = 0\n\n";

    cout << "  Sa log space:\n";
    cout << "  2×ln(φ) = ln(φ + 1)\n\n";

    cout << "  Kaya ang 2×ln(φ) ay nagre-represent ng\n";
    cout << "  φ² = φ + 1, hindi 2φ\n\n";

    // ============================================
    // BEATTY MODULO 2
    // ============================================

    cout << "========================================\n";
    cout << "  BEATTY MODULO 2\n";
    cout << "========================================\n\n";

    cout << "  Ang Beatty sequence ⌊nφ⌋ ay may periodicity:\n";
    cout << "  n:  1  2  3  4  5  6  7  8  9 10\n";
    cout << "  φ:  1  3  4  6  8  9 11 12 14 16\n";
    cout << "  mod2: 1  1  0  0  0  1  1  0  0  0\n\n";

    cout << "  Ang Beatty sequence ⌊nφ²⌋ ay may periodicity:\n";
    cout << "  n:    1  2  3  4  5  6  7  8  9 10\n";
    cout << "  φ²:   2  5  7 10 13 15 18 20 23 26\n";
    cout << "  mod2: 0  1  1  0  1  1  0  0  1  0\n\n";

    // ============================================
    // ANG KEY
    // ============================================

    cout << "========================================\n";
    cout << "  ANG KEY\n";
    cout << "========================================\n\n";

    cout << "  Ang XOR ay modulo 2.\n";
    cout << "  Ang Beatty sequence ⌊nφ⌋ ay may natural na\n";
    cout << "  modulo 2 pattern na nagre-repeat bawat 3 steps:\n";
    cout << "  ⌊nφ⌋ mod 2: 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, ...\n\n";

    cout << "  Ito ay period-5 pattern!\n";
    cout << "  1, 1, 0, 0, 0 | 1, 1, 0, 0, 0 | ...\n\n";

    cout << "  Ang period-5 na ito ay φ-related:\n";
    cout << "  φ⁵ = 11.090... ≈ 11\n";
    cout << "  ⌊5φ⌋ = 8\n";
    cout << "  ⌊5φ²⌋ = 13\n\n";

    // ============================================
    // RULE 110 VIA BEATTY MODULO
    // ============================================

    cout << "========================================\n";
    cout << "  RULE 110 VIA BEATTY MODULO\n";
    cout << "========================================\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "  L C R | Binary | φ-floor | φ-floor mod 2 | Expected | Match?\n";
    cout << "  ------|--------|---------|---------------|----------|--------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                double val = pattern * PHI;
                int floor_val = (int)floor(val);
                int mod2 = floor_val % 2;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(6) << pattern << " | "
                     << setw(7) << floor_val << " | "
                     << setw(13) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (mod2 == expected ? "✅" : "❌") << "\n";
            }
        }
    }

    return 0;
}
