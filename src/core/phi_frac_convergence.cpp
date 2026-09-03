// ============================================
// φ-FRACTIONAL CONVERGENCE TEST
// Hanapin ang formula para sa φ^frac
// na hindi nangangailangan ng exponentiation
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
    cout << "  φ-FRACTIONAL CONVERGENCE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double PHI_INV = 1.0 / PHI;

    // ============================================
    // TEST 1: FRAC CONVERGENCE
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: FRAC CONVERGENCE\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | frac | φ^frac | φ^frac - 1 | Ratio\n";
    cout << "  --|-----------|------|---------|-------------|-------\n";

    double prev_frac = 0.0;
    double prev_phi_frac = 1.0;

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0, 89.0, 144.0, 233.0, 377.0, 610.0, 987.0, 1597.0, 2584.0, 4181.0, 6765.0}) {
        double log_phi_x = log(x) / LN_PHI;
        double frac = log_phi_x - floor(log_phi_x);
        double phi_frac = pow(PHI, frac);
        
        double diff = phi_frac - 1.0;
        double ratio = (prev_phi_frac > 0) ? phi_frac / prev_phi_frac : 0;
        
        cout << "  " << setw(5) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(5) << frac << " | "
             << setw(7) << phi_frac << " | "
             << setw(11) << diff << " | "
             << setw(5) << ratio << "\n";
        
        prev_frac = frac;
        prev_phi_frac = phi_frac;
    }

    // ============================================
    // TEST 2: FRAC ≈ log_φ(φ^frac) ?
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 2: FRAC → φ^frac RELATION\n";
    cout << "========================================\n\n";

    cout << "  frac | φ^frac | φ^frac - 1 | (φ^frac - 1)/(φ-1)\n";
    cout << "  -----|---------|------------|-------------------\n";

    for (double frac : {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9}) {
        double phi_frac = pow(PHI, frac);
        double normalized = (phi_frac - 1.0) / (PHI - 1.0);
        
        cout << "  " << setw(5) << fixed << setprecision(1) << frac << " | "
             << setw(7) << fixed << setprecision(4) << phi_frac << " | "
             << setw(10) << (phi_frac - 1.0) << " | "
             << setw(17) << normalized << "\n";
    }

    // ============================================
    // TEST 3: FIBONACCI RATIO ANALYSIS
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 3: FIBONACCI RATIO\n";
    cout << "========================================\n\n";

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  n | F_n | F_n / φ^n | frac part\n";
    cout << "  --|-----|-----------|----------\n";

    for (int n = 1; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double ratio = fib[n] / phi_n;
        double frac = ratio - floor(ratio);
        
        cout << "  " << setw(2) << n << " | "
             << setw(4) << fib[n] << " | "
             << setw(9) << fixed << setprecision(6) << ratio << " | "
             << setw(8) << frac << "\n";
    }

    cout << "\n========================================\n";
    cout << "  KEY OBSERVATION\n";
    cout << "========================================\n\n";
    cout << "  Ang fractional part ay may convergence pattern.\n";
    cout << "  Ang Fibonacci ratio ay may exact formula:\n";
    cout << "  F_n / φ^n = (1 - (-φ)^{-2n}) / √5\n\n";

    return 0;
}
