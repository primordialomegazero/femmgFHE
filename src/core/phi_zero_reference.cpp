// ============================================
// φ-ZERO REFERENCE
// Subok kung ang zero ay may espesyal na papel
// sa bridge sa pagitan ng log at normal space
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
    cout << "  φ-ZERO REFERENCE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  log_φ(1) = 0\n";
    cout << "  log_φ(φ) = 1\n";
    cout << "  log_φ(φ²) = 2\n\n";

    // ============================================
    // TEST: ZERO-POSITION SA LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  ZERO-POSITION SA LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | log_φ(x) - floor | φ^(log_φ(x) - floor)\n";
    cout << "  --|-----------|------------------|------------------------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 35.0, 55.0}) {
        double log_phi_x = log(x) / LN_PHI;
        double floor_val = floor(log_phi_x);
        double frac = log_phi_x - floor_val;
        double phi_frac = pow(PHI, frac);
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(16) << frac << " | "
             << setw(22) << phi_frac << "\n";
    }

    // ============================================
    // TEST: ZERO-REFERENCED ENCODING
    // ============================================

    cout << "\n========================================\n";
    cout << "  ZERO-REFERENCED ENCODING\n";
    cout << "========================================\n\n";

    cout << "  Subok: i-encode ang value bilang:\n";
    cout << "  v = x - floor(x/φ) × φ\n";
    cout << "  (zero-referenced sa φ multiples)\n\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 35.0, 55.0}) {
        double k = floor(x / PHI);
        double ref = x - k * PHI;
        
        cout << "  " << setw(3) << x << " | "
             << "k=" << setw(2) << k << " | "
             << "x - kφ = " << fixed << setprecision(4) << ref << "\n";
    }

    // ============================================
    // TEST: ZERO BRIDGE SA (5 × 7) + 3
    // ============================================

    cout << "\n========================================\n";
    cout << "  ZERO BRIDGE SA (5 × 7) + 3\n";
    cout << "========================================\n\n";

    // 35 sa log space
    double log_35 = log(35.0) / LN_PHI;
    double log_5 = log(5.0) / LN_PHI;
    double log_7 = log(7.0) / LN_PHI;
    double log_3 = log(3.0) / LN_PHI;

    cout << "  log_φ(5) = " << log_5 << "\n";
    cout << "  log_φ(7) = " << log_7 << "\n";
    cout << "  log_φ(5) + log_φ(7) = " << log_35 << "\n\n";

    // Subok: i-zero-reference ang log values
    double ref_5 = log_5 - floor(log_5);
    double ref_7 = log_7 - floor(log_7);
    double ref_35 = log_35 - floor(log_35);

    cout << "  Zero-referenced:\n";
    cout << "  ref_5: " << ref_5 << " → φ^" << ref_5 << " = " << pow(PHI, ref_5) << "\n";
    cout << "  ref_7: " << ref_7 << " → φ^" << ref_7 << " = " << pow(PHI, ref_7) << "\n";
    cout << "  ref_35: " << ref_35 << " → φ^" << ref_35 << " = " << pow(PHI, ref_35) << "\n\n";

    // Subok: (φ^ref_5 × φ^ref_7) = φ^ref_35?
    double prod_ref = pow(PHI, ref_5) * pow(PHI, ref_7);
    cout << "  φ^ref_5 × φ^ref_7 = " << prod_ref << "\n";
    cout << "  φ^ref_35 = " << pow(PHI, ref_35) << "\n";
    cout << "  Match: " << (abs(prod_ref - pow(PHI, ref_35)) < 0.01 ? "✅" : "❌") << "\n\n";

    return 0;
}
