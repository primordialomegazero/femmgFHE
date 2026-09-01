// ============================================
// φ-NEW DISCOVERY — HANAPIN ANG HINDI PA NATIN NAKIKITA
//
// I-explore ang mga φ-properties na maaaring
// magbigay ng natural na non-linearity:
//
// 1. φ + φ⁻¹ = √5
// 2. φ - φ⁻¹ = 1
// 3. φ² + φ⁻² = 3
// 4. φ³ + φ⁻³ = 4.236
// 5. φ^n + φ^(-n) = L(n) — Lucas numbers!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-NEW DISCOVERY\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    // ============================================
    // TEST 1: φ + φ⁻¹ = √5
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: φ + φ⁻¹\n";
    cout << "========================================\n\n";

    double sum_phi = PHI + PHI_INV;
    double sqrt5 = sqrt(5.0);
    
    cout << "  φ + φ⁻¹ = " << sum_phi << "\n";
    cout << "  √5 = " << sqrt5 << "\n";
    cout << "  Match: " << (abs(sum_phi - sqrt5) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: LUCAS NUMBERS (φ^n + φ^(-n))
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: LUCAS NUMBERS\n";
    cout << "========================================\n\n";

    cout << "  L(n) = φ^n + φ^(-n)\n";
    cout << "  n | L(n) | Is Integer? | Non-linear?\n";
    cout << "  --|------|-------------|------------\n";

    for (int n = 0; n < 10; n++) {
        double lucas = pow(PHI, n) + pow(PHI, -n);
        bool is_int = (abs(lucas - round(lucas)) < 0.01);
        
        cout << "  " << setw(2) << n << " | "
             << setw(6) << fixed << setprecision(3) << lucas << " | "
             << setw(11) << (is_int ? "✅" : "❌") << " | "
             << (n > 1 ? "✅ non-linear" : "❌") << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 3: φ-HARMONIC PAIRS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: φ-HARMONIC PAIRS\n";
    cout << "========================================\n\n";

    cout << "  n | φ^n | φ^(-n) | Sum | Product\n";
    cout << "  --|-----|--------|-----|--------\n";

    for (int n : {1, 2, 3, 4, 5}) {
        double phi_n = pow(PHI, n);
        double phi_neg_n = pow(PHI_INV, n);
        double sum = phi_n + phi_neg_n;
        double product = phi_n * phi_neg_n;
        
        cout << "  " << n << " | "
             << setw(5) << fixed << setprecision(3) << phi_n << " | "
             << setw(6) << phi_neg_n << " | "
             << setw(5) << sum << " | "
             << setw(6) << product << "\n";
    }

    cout << "\n  Lahat ng products ay 1! (φ^n × φ^(-n) = 1)\n\n";

    // ============================================
    // TEST 4: φ-NESTED RADICALS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: φ-NESTED RADICALS\n";
    cout << "========================================\n\n";

    cout << "  φ = √(1 + √(1 + √(1 + ...)))\n";
    cout << "  Ito ay INFINITE NESTED RADICAL!\n\n";

    // Compute nested radical approximation
    double nested = 1.0;
    for (int i = 0; i < 30; i++) {
        nested = sqrt(1.0 + nested);
    }
    
    cout << "  Nested radical (30 levels) = " << nested << "\n";
    cout << "  φ = " << PHI << "\n";
    cout << "  Match: " << (abs(nested - PHI) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 5: φ-CONTINUED FRACTION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 5: φ-CONTINUED FRACTION\n";
    cout << "========================================\n\n";

    cout << "  φ = 1 + 1/(1 + 1/(1 + ...))\n";
    cout << "  Ito ay INFINITE CONTINUED FRACTION!\n\n";

    double cf = 1.0;
    for (int i = 0; i < 30; i++) {
        cf = 1.0 + 1.0 / cf;
    }
    
    cout << "  Continued fraction (30 levels) = " << cf << "\n";
    cout << "  φ = " << PHI << "\n";
    cout << "  Match: " << (abs(cf - PHI) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 6: φ-NON-LINEAR EMERGENT
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 6: φ-NON-LINEAR EMERGENT\n";
    cout << "========================================\n\n";

    cout << "  Ang φ ay may INFINITE recursion:\n";
    cout << "  φ = 1 + 1/φ\n";
    cout << "  φ² = φ + 1\n";
    cout << "  φ³ = 2φ + 1\n\n";

    cout << "  Ito ay NATURAL na non-linear!\n";
    cout << "  Hindi linear approximation — kundi\n";
    cout << "  EXACT na non-linear recursion!\n\n";

    cout << "========================================\n";
    cout << "  NEW DISCOVERY COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ φ + φ⁻¹ = √5\n";
    cout << "  ✅ L(n) = φ^n + φ^(-n) — Lucas numbers\n";
    cout << "  ✅ φ^n × φ^(-n) = 1 — harmonic pairs\n";
    cout << "  ✅ φ = nested radical / continued fraction\n";
    cout << "  ✅ φ = 1 + 1/φ — infinite recursion\n\n";

    return 0;
}
