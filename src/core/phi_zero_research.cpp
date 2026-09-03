// ============================================
// φ-ZERO RESEARCH
// Hanapin ang papel ng zero sa φ-power space
// at kung paano ito makakatulong sa bridge
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
    cout << "  φ-ZERO RESEARCH\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // ZERO SA φ-POWER SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  ZERO SA φ-POWER SPACE\n";
    cout << "========================================\n\n";

    cout << "  φ^0 = 1\n";
    cout << "  φ^1 = φ\n";
    cout << "  φ^2 = φ + 1\n\n";

    cout << "  Ang zero ay may special na papel:\n";
    cout << "  φ^0 = 1 (hindi zero!)\n";
    cout << "  Walang φ^n = 0\n\n";

    cout << "  Pero may zero sa normal space:\n";
    cout << "  0 = ? sa φ-power space\n";
    cout << "  Walang φ^n = 0 → hindi ma-encode ang 0\n\n";

    // ============================================
    // ZERO-REFERENCE SA φ-POWER SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  ZERO-REFERENCE SA φ-POWER SPACE\n";
    cout << "========================================\n\n";

    cout << "  Subok: i-reference ang values sa φ^0 = 1\n";
    cout << "  x → φ^n - φ^0 = φ^n - 1\n";
    cout << "  Para sa 0: φ^? - φ^0 = 0\n\n";

    cout << "  x | φ^n | φ^n - φ^0 | x-1\n";
    cout << "  --|-----|-----------|----\n";

    for (double x : {1.0, 2.0, 3.0, 5.0, 8.0, 13.0}) {
        double n = log(x) / LN_PHI;
        double phi_n = pow(PHI, n);
        double phi_ref = phi_n - 1.0;
        
        cout << "  " << setw(3) << x << " | "
             << setw(7) << fixed << setprecision(4) << phi_n << " | "
             << setw(9) << phi_ref << " | "
             << setw(5) << (x - 1) << "\n";
    }

    // ============================================
    // ZERO ANGLE — GOLDEN ANGLE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ZERO ANGLE — GOLDEN ANGLE\n";
    cout << "========================================\n\n";

    cout << "  Golden angle = 2π/φ² = 137.5°\n";
    cout << "  Ito ay natural na rotation na bumabalik sa zero\n\n";

    double golden_angle = 2.0 * M_PI / (PHI * PHI);
    cout << "  Golden angle (radians): " << golden_angle << "\n";
    cout << "  Golden angle (degrees): " << golden_angle * 180.0 / M_PI << "\n\n";

    // ============================================
    // ZERO SA INDEX SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  ZERO SA INDEX SPACE\n";
    cout << "========================================\n\n";

    cout << "  Sa index space, ang zero ay may meaning:\n";
    cout << "  n = 0 → φ^0 = 1\n";
    cout << "  n < 0 → φ^n < 1\n";
    cout << "  n > 0 → φ^n > 1\n\n";

    cout << "  n | φ^n | φ^n - 1\n";
    cout << "  --|-----|--------\n";

    for (int n : {-3, -2, -1, 0, 1, 2, 3}) {
        double phi_n = pow(PHI, n);
        cout << "  " << setw(2) << n << " | "
             << setw(7) << fixed << setprecision(4) << phi_n << " | "
             << setw(7) << (phi_n - 1.0) << "\n";
    }

    // ============================================
    // ZERO BRIDGE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ZERO BRIDGE\n";
    cout << "========================================\n\n";

    cout << "  Subok: zero bilang bridge\n";
    cout << "  0 → φ^? (walang exact na φ^n = 0)\n\n";

    cout << "  Pero may limit:\n";
    cout << "  φ^(-∞) → 0\n";
    cout << "  Kaya ang 0 ay nasa -∞ sa φ-power space\n\n";

    cout << "  Ito ay may espesyal na property:\n";
    cout << "  0 + x = x (sa normal space)\n";
    cout << "  φ^(-∞) + φ^n = φ^n (sa power space)\n\n";

    // ============================================
    // KEY INSIGHT
    // ============================================

    cout << "========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n\n";

    cout << "  1. Ang zero ay walang exact representation sa φ-power space\n";
    cout << "  2. Pero ang φ^0 = 1 ay ang 'unity' reference\n";
    cout << "  3. Ang zero-reference (x - 1) ay pwedeng magbigay ng natural na bridge\n";
    cout << "  4. Ang golden angle ay nagbibigay ng natural na periodicity\n\n";

    return 0;
}
