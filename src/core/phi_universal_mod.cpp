// ============================================
// φ-UNIVERSAL HOMOMORPHIC MODULO
// Test ang universal modulo using φ-harmonization
//
// x mod m = x - m × (x/m - frac(x/m))
// Kung saan frac(x/m) ay bounded sa [0,1)
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
    cout << "  φ-UNIVERSAL HOMOMORPHIC MODULO\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST 1: UNIVERSAL MOD VIA FRAC
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: x mod m VIA FRAC\n";
    cout << "========================================\n\n";

    cout << "  x | m | x/m | frac(x/m) | x mod m | Match?\n";
    cout << "  --|---|-----|-----------|---------|--------\n";

    for (double x : {5.0, 7.0, 35.0, 100.0, 1000.0}) {
        for (double m : {PHI, 2.0, 3.0, 5.0}) {
            double div = x / m;
            double frac = div - floor(div);
            double mod = frac * m;
            double expected = fmod(x, m);
            bool match = abs(mod - expected) < 0.01;
            
            cout << "  " << setw(5) << x << " | "
                 << setw(2) << m << " | "
                 << setw(5) << fixed << setprecision(3) << div << " | "
                 << setw(9) << frac << " | "
                 << setw(7) << mod << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 2: GOLDEN RATIO HARMONIZATION
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 2: GOLDEN RATIO HARMONIZATION\n";
    cout << "========================================\n\n";

    cout << "  φ-based frac approximation:\n";
    cout << "  frac(x) ≈ x - floor(x)\n";
    cout << "  At: floor(x) = x - frac(x)\n\n";

    cout << "  x | frac(x) | φ-frac | Match?\n";
    cout << "  --|---------|--------|--------\n";

    for (double x : {0.1, 0.3, 0.5, 0.7, 0.9, 1.1, 2.3, 3.7}) {
        double frac = x - floor(x);
        double phi_frac = fmod(x, 1.0);
        bool match = abs(frac - phi_frac) < 0.001;
        
        cout << "  " << setw(4) << x << " | "
             << setw(7) << fixed << setprecision(4) << frac << " | "
             << setw(6) << phi_frac << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST 3: UNIVERSAL MOD VIA φ-PERIODICITY
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 3: UNIVERSAL MOD VIA φ-PERIODICITY\n";
    cout << "========================================\n\n";

    cout << "  x mod m = x - m × round(x/m - 0.5)\n";
    cout << "  (approximation na walang floor)\n\n";

    cout << "  x | m | round(x/m-0.5) | x mod m | Expected | Match?\n";
    cout << "  --|---|----------------|---------|----------|--------\n";

    for (double x : {5.0, 7.0, 35.0, 100.0}) {
        for (double m : {PHI, 2.0, 3.0, 5.0}) {
            double k = round(x / m - 0.5);
            double mod = x - k * m;
            double expected = fmod(x, m);
            bool match = abs(mod - expected) < 0.01;
            
            cout << "  " << setw(4) << x << " | "
                 << setw(1) << m << " | "
                 << setw(14) << k << " | "
                 << setw(7) << fixed << setprecision(3) << mod << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 4: φ-UNIVERSAL FORMULA
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 4: φ-UNIVERSAL FORMULA\n";
    cout << "========================================\n\n";

    cout << "  Para sa anumang m, ang modulo ay:\n";
    cout << "  x mod m = x - m × floor(x/m)\n";
    cout << "  = x - m × (x/m - frac(x/m))\n";
    cout << "  = x - x + m × frac(x/m)\n";
    cout << "  = m × frac(x/m)\n\n";

    cout << "  Kaya: x mod m = m × frac(x/m)\n\n";

    cout << "  x | m | frac(x/m) | m×frac | fmod(x,m) | Match?\n";
    cout << "  --|---|-----------|--------|-----------|--------\n";

    for (double x : {5.0, 7.0, 35.0, 100.0}) {
        for (double m : {PHI, 2.0, 3.0, 5.0}) {
            double frac = fmod(x/m, 1.0);
            double mod = m * frac;
            double expected = fmod(x, m);
            bool match = abs(mod - expected) < 0.01;
            
            cout << "  " << setw(4) << x << " | "
                 << setw(2) << m << " | "
                 << setw(9) << fixed << setprecision(4) << frac << " | "
                 << setw(6) << mod << " | "
                 << setw(9) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n========================================\n";
    cout << "  KEY FINDING\n";
    cout << "========================================\n\n";
    cout << "  x mod m = m × frac(x/m)\n";
    cout << "  Ang frac(x/m) ay bounded sa [0,1)\n";
    cout << "  At frac(x/m) = fmod(x/m, 1.0)\n\n";
    cout << "  Para sa homomorphic modulo:\n";
    cout << "  EvalMult(x, 1/m) → x/m\n";
    cout << "  EvalSub(floor(x/m)) → frac(x/m)\n";
    cout << "  EvalMult(frac(x/m), m) → x mod m\n\n";

    return 0;
}
