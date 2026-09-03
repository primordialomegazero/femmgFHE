// ============================================
// φ-HARMONIZE
// I-sync ang normal at log space
// gamit ang golden ratio harmonization
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
    cout << "  φ-HARMONIZE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // HARMONIZATION: Normal ↔ Log
    // ============================================

    cout << "========================================\n";
    cout << "  HARMONIZATION: Normal ↔ Log\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | n | frac | φ^n | φ^frac | Reconstructed\n";
    cout << "  --|-----------|----|------|-----|--------|-------------\n";

    for (double x : {5.0, 7.0, 35.0, 105.0}) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_n = fib[n-1] + fib[n] * PHI;
        double phi_frac = pow(PHI, frac);
        double recon = phi_n * phi_frac;
        
        cout << "  " << setw(4) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(2) << n << " | "
             << setw(5) << frac << " | "
             << setw(6) << phi_n << " | "
             << setw(7) << phi_frac << " | "
             << setw(10) << recon << "\n";
    }

    // ============================================
    // SYNCHRONIZATION
    // ============================================

    cout << "\n========================================\n";
    cout << "  SYNCHRONIZATION\n";
    cout << "========================================\n\n";

    cout << "  Ang normal space (Slot 3) ay kailangang\n";
    cout << "  i-sync sa log space (Slot 2)\n\n";

    cout << "  Sync formula: x = φ^(log_φ(x))\n";
    cout << "  = φ^n × φ^frac\n";
    cout << "  = (F_{n-1} + F_n×φ) × φ^frac\n\n";

    cout << "  Ito ay pwedeng gawin sa:\n";
    cout << "  - EvalAdd para sa F_{n-1} + F_n×φ\n";
    cout << "  - EvalMult(constant) para sa × φ^frac\n";
    cout << "  O natural na φ-harmonization\n\n";

    // ============================================
    // GOLDEN RATIO HARMONIZATION
    // ============================================

    cout << "========================================\n";
    cout << "  GOLDEN RATIO HARMONIZATION\n";
    cout << "========================================\n\n";

    cout << "  φ² = φ + 1\n";
    cout << "  Ito ay natural na harmonization:\n";
    cout << "  φ^n = φ^(n-1) + φ^(n-2)\n\n";

    cout << "  n | φ^n | φ^(n-1) + φ^(n-2)\n";
    cout << "  --|-----|------------------\n";

    for (int n = 2; n <= 10; n++) {
        double phi_n = pow(PHI, n);
        double phi_n1 = pow(PHI, n-1);
        double phi_n2 = pow(PHI, n-2);
        
        cout << "  " << setw(2) << n << " | "
             << setw(7) << fixed << setprecision(4) << phi_n << " | "
             << setw(14) << (phi_n1 + phi_n2) << "\n";
    }

    return 0;
}
