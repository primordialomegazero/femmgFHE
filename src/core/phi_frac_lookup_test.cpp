// ============================================
// φ-FRACTIONAL LOOKUP TEST
// Ang φ^frac ay bounded sa [1, φ)
// Pre-compute ang fractional lookup table
// para sa conversion nang walang EvalMult
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-FRACTIONAL LOOKUP TEST\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST: FRACTIONAL PART BOUNDED SA [0, 1)
    // ============================================

    cout << "========================================\n";
    cout << "  FRACTIONAL PART ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  log_φ(x) | frac | φ^frac | Pattern?\n";
    cout << "  ----------|------|---------|----------\n";

    vector<double> fracs;
    vector<double> phi_fracs;

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0, 89.0, 144.0}) {
        double log_phi_x = log(x) / LN_PHI;
        double frac = log_phi_x - floor(log_phi_x);
        double phi_frac = pow(PHI, frac);
        
        fracs.push_back(frac);
        phi_fracs.push_back(phi_frac);
        
        cout << "  " << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(5) << frac << " | "
             << setw(7) << phi_frac << " | "
             << (abs(phi_frac - PHI) < 0.01 ? "≈φ" : 
                 abs(phi_frac - 1.0) < 0.01 ? "≈1" : "") << "\n";
    }

    // ============================================
    // TEST: φ^frac ≈ 1 + frac × (φ - 1) ??
    // ============================================

    cout << "\n========================================\n";
    cout << "  φ^frac vs LINEAR APPROXIMATION\n";
    cout << "========================================\n\n";

    cout << "  frac | φ^frac | 1+frac×(φ-1) | Err | Match?\n";
    cout << "  -----|---------|--------------|-----|--------\n";

    for (size_t i = 0; i < fracs.size(); i++) {
        double linear = 1.0 + fracs[i] * (PHI - 1.0);
        double err = abs(phi_fracs[i] - linear);
        bool match = err < 0.01;
        
        cout << "  " << setw(5) << fixed << setprecision(4) << fracs[i] << " | "
             << setw(7) << phi_fracs[i] << " | "
             << setw(12) << linear << " | "
             << setw(5) << err << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST: LOG → NORMAL VIA FRAC LOOKUP
    // ============================================

    cout << "\n========================================\n";
    cout << "  LOG → NORMAL VIA FRAC LOOKUP\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | int | frac | φ^int | φ^frac | Recovered | Match?\n";
    cout << "  --|-----------|-----|------|-------|--------|-----------|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 35.0, 100.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int int_part = (int)floor(log_phi_x);
        double frac_part = log_phi_x - int_part;
        
        double phi_int = pow(PHI, int_part);
        double phi_frac = pow(PHI, frac_part);
        double recovered = phi_int * phi_frac;
        bool match = abs(recovered - x) < 0.01;
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(3) << int_part << " | "
             << setw(5) << frac_part << " | "
             << setw(5) << phi_int << " | "
             << setw(6) << phi_frac << " | "
             << setw(9) << recovered << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n\n";
    cout << "  Ang φ^frac ay BOUNDED sa [1, φ).\n";
    cout << "  Kung may lookup table tayo para sa φ^frac,\n";
    cout << "  at Fibonacci para sa φ^int,\n";
    cout << "  baka kaya nating i-convert ang log papuntang\n";
    cout << "  normal nang walang EvalMult.\n\n";

    return 0;
}
