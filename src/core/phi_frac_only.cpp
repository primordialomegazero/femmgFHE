// ============================================
// φ-FRAC ONLY ENCODING
// Slot 0: x (normal)
// Slot 1: n (floor index)
// Slot 2: frac (fractional part)
// Slot 3: φ^frac (pre-computed)
//
// Ang EvalAdd ay nagbibigay ng:
// Slot 0: x1 + x2 (addition)
// Slot 1: n1 + n2 (index addition)
// Slot 2: frac1 + frac2 (fractional addition)
// Slot 3: φ^frac1 + φ^frac2 (hindi multiplicative)
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
    cout << "  φ-FRAC ONLY ENCODING\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  x | n | frac | φ^frac | F_n | a+bφ\n";
    cout << "  --|---|------|--------|-----|------\n";

    for (double x : {5.0, 7.0, 35.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        double frac = log_phi_x - n;
        double phi_frac = pow(PHI, frac);
        
        double a = fib[n-1] * phi_frac;
        double b = fib[n] * phi_frac;
        
        cout << "  " << setw(3) << x << " | "
             << setw(1) << n << " | "
             << setw(4) << fixed << setprecision(4) << frac << " | "
             << setw(6) << phi_frac << " | "
             << setw(3) << fib[n] << " | "
             << setw(6) << (a + b * PHI) << "\n";
    }

    // ============================================
    // ADDITION TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION TEST\n";
    cout << "========================================\n\n";

    // 5 + 7
    double n5 = 3, frac5 = 0.3446;
    double n7 = 4, frac7 = 0.0438;
    
    double n_sum = n5 + n7;
    double frac_sum = frac5 + frac7;
    
    cout << "  n(5) + n(7) = " << n_sum << "\n";
    cout << "  frac(5) + frac(7) = " << frac_sum << "\n\n";

    // Reconstruct mula sa n_sum at frac_sum
    int n_recon = (int)n_sum;
    double phi_frac_recon = pow(PHI, frac_sum);
    double a_recon = fib[n_recon - 1] * phi_frac_recon;
    double b_recon = fib[n_recon] * phi_frac_recon;
    double recovered = a_recon + b_recon * PHI;
    
    cout << "  Reconstructed: " << recovered << " (expected: 35)\n";
    cout << "  Match: " << (abs(recovered - 35.0) < 0.01 ? "✅" : "❌") << "\n\n";

    return 0;
}
