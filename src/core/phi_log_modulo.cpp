// ============================================
// φ-LOG MODULO
// Ang modulo sa log space para ma-bound
// ang values at walang overflow
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
    cout << "  φ-LOG MODULO\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST 1: LOG MODULO PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  LOG MODULO PATTERN\n";
    cout << "========================================\n\n";

    cout << "  log_φ(x) | fmod(log, φ) | φ^fmod(log) | φ^log mod φ\n";
    cout << "  ---------|--------------|-------------|------------\n";

    for (double x : {5.0, 7.0, 35.0, 100.0, 500.0, 1000.0}) {
        double log_phi_x = log(x) / LN_PHI;
        double mod_log = fmod(log_phi_x, PHI);
        double phi_mod = pow(PHI, mod_log);
        double phi_log_mod = fmod(pow(PHI, log_phi_x), PHI);
        
        cout << "  " << setw(8) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(12) << mod_log << " | "
             << setw(11) << phi_mod << " | "
             << setw(10) << phi_log_mod << "\n";
    }

    // ============================================
    // TEST 2: PERIODICITY NG LOG MODULO
    // ============================================

    cout << "\n========================================\n";
    cout << "  PERIODICITY NG LOG MODULO\n";
    cout << "========================================\n\n";

    cout << "  φ^n mod φ:\n";
    cout << "  n | φ^n | fmod(φ^n, φ)\n";
    cout << "  --|-----|-------------\n";

    for (int n = 0; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double mod_phi = fmod(phi_n, PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(8) << fixed << setprecision(4) << phi_n << " | "
             << setw(9) << mod_phi << "\n";
    }

    // ============================================
    // TEST 3: LOG MODULO FORMULA
    // ============================================

    cout << "\n========================================\n";
    cout << "  LOG MODULO FORMULA\n";
    cout << "========================================\n\n";

    cout << "  φ^(log_φ(x) mod φ) = φ^(log_φ(x)) mod φ\n";
    cout << "  Kasi φ^φ ay periodic\n\n";

    cout << "  φ^φ = " << pow(PHI, PHI) << "\n";
    cout << "  fmod(φ^φ, φ) = " << fmod(pow(PHI, PHI), PHI) << "\n\n";

    cout << "  May periodicity ba?\n";
    cout << "  φ^(n+φ) mod φ = ?\n\n";

    for (int n = 0; n <= 5; n++) {
        double phi_n = pow(PHI, n + PHI);
        double mod_phi = fmod(phi_n, PHI);
        cout << "  φ^" << n << "+φ mod φ = " << mod_phi << "\n";
    }

    return 0;
}
