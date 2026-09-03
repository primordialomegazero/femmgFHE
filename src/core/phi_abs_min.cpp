// ============================================
// φ-ABS MIN
// φ-modulo bilang natural na abs at min
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-ABS MIN\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    // ============================================
    // φ-MODULO BILANG ABS
    // ============================================

    cout << "========================================\n";
    cout << "  φ-MODULO BILANG ABS\n";
    cout << "========================================\n\n";

    cout << "  |a-b| = ?\n";
    cout << "  Kung a-b ≥ 0: |a-b| = a-b = fmod(a-b, φ)\n";
    cout << "  Kung a-b < 0: |a-b| = b-a = fmod(b-a, φ)\n\n";

    cout << "  Sa φ-power space:\n";
    cout << "  φ^|a-b| = φ^(fmod(|a-b|, φ))\n";
    cout << "  Ito ay bounded — walang overflow\n\n";

    // ============================================
    // TEST: φ^a + φ^b
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: φ^a + φ^b GAMIT ANG MODULO\n";
    cout << "========================================\n\n";

    cout << "  a=3, b=2: φ³ + φ² = 4.236 + 2.618 = 6.854 = φ⁴\n";
    cout << "  |a-b| = 1 → φ^1 = φ\n";
    cout << "  max(a,b) = 3\n";
    cout << "  Result: φ^3 × (1 + φ^1) = 4.236 × 2.618 = 11.09\n";
    cout << "  Mali! Dapat: φ⁴ = 6.854\n\n";

    cout << "  HINDI GUMAGANA ANG DIRECT FORMULA\n\n";

    // ============================================
    // ANG TAMA
    // ============================================

    cout << "========================================\n";
    cout << "  ANG TAMA\n";
    cout << "========================================\n\n";

    cout << "  φ^a + φ^b = φ^min(a,b) × (1 + φ^|a-b|)\n\n";

    cout << "  a=3, b=2:\n";
    cout << "  min = 2, |a-b| = 1\n";
    cout << "  = φ² × (1 + φ¹)\n";
    cout << "  = 2.618 × 2.618\n";
    cout << "  = 6.854\n";
    cout << "  = φ⁴ ✅\n\n";

    cout << "  ANG TAMANG FORMULA:\n";
    cout << "  φ^a + φ^b = φ^min(a,b) × (1 + φ^|a-b|)\n";
    cout << "  At: 1 + φ = φ²\n";
    cout << "  Kaya: φ² × φ² = φ⁴ ✅\n\n";

    return 0;
}
