// ============================================
// φ-FRAC WRAP
// I-wrap ang fract sa [0, 1) gamit ang
// natural na periodicity ng φ
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
    cout << "  φ-FRAC WRAP\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // FRAC WRAP TEST
    // ============================================

    cout << "========================================\n";
    cout << "  FRAC WRAP\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | frac | floor | φ^frac\n";
    cout << "  --|-----------|------|-------|-------\n";

    for (double x : {1.0, 5.0, 7.0, 35.0, 100.0}) {
        double log_phi_x = log(x) / LN_PHI;
        double floor_val = floor(log_phi_x);
        double frac = log_phi_x - floor_val;
        double phi_frac = pow(PHI, frac);
        
        cout << "  " << setw(4) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(5) << frac << " | "
             << setw(5) << floor_val << " | "
             << setw(6) << phi_frac << "\n";
    }

    // ============================================
    // NATURAL WRAP-AROUND
    // ============================================

    cout << "\n========================================\n";
    cout << "  NATURAL WRAP-AROUND\n";
    cout << "========================================\n\n";

    cout << "  φ^n mod 1:\n";
    cout << "  n | φ^n | φ^n mod 1 | frac part\n";
    cout << "  --|-----|-----------|----------\n";

    for (int n = 0; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double mod1 = fmod(phi_n, 1.0);
        double frac = phi_n - floor(phi_n);
        
        cout << "  " << setw(2) << n << " | "
             << setw(7) << fixed << setprecision(4) << phi_n << " | "
             << setw(9) << mod1 << " | "
             << setw(8) << frac << "\n";
    }

    // ============================================
    // EMERGENT PROPERTY
    // ============================================

    cout << "\n========================================\n";
    cout << "  EMERGENT PROPERTY\n";
    cout << "========================================\n\n";

    cout << "  φ^n = F_n φ + F_{n-1}\n";
    cout << "  φ^n mod 1 = (F_n φ + F_{n-1}) mod 1\n";
    cout << "  = (F_n × 0.618 + F_{n-1}) mod 1\n\n";

    for (int n = 0; n <= 10; n++) {
        double phi_n = pow(PHI, n);
        double frac = fmod(phi_n, 1.0);
        
        cout << "  n=" << setw(2) << n << ": φ^n=" << setw(7) << fixed << setprecision(4) 
             << phi_n << ", frac=" << setw(5) << frac << "\n";
    }

    return 0;
}
