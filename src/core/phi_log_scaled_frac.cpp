// ============================================
// φ-LOG SCALED FRAC
// Slot 0: x
// Slot 1: n
// Slot 2: frac
// Slot 3: ln(φ^frac) = frac × ln(φ)
//
// Ang EvalAdd ay nagbibigay ng:
// Slot 1: n1 + n2 (tamang index)
// Slot 2: frac1 + frac2 (tamang fract)
// Slot 3: frac1×ln(φ) + frac2×ln(φ) = frac_sum×ln(φ)
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
    cout << "  φ-LOG SCALED FRAC\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  x | n | frac | frac×ln(φ) | F_n\n";
    cout << "  --|---|------|-----------|-----\n";

    for (double x : {5.0, 7.0, 35.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        double frac = log_phi_x - n;
        double log_scaled = frac * LN_PHI;
        
        cout << "  " << setw(3) << x << " | "
             << setw(1) << n << " | "
             << setw(4) << fixed << setprecision(4) << frac << " | "
             << setw(9) << log_scaled << " | "
             << setw(3) << fib[n] << "\n";
    }

    // ============================================
    // ADDITION TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION TEST\n";
    cout << "========================================\n\n";

    double n5 = 3, frac5 = 0.3446;
    double n7 = 4, frac7 = 0.0438;
    
    double n_sum = n5 + n7;
    double frac_sum = frac5 + frac7;
    double log_scaled_sum = frac5 * LN_PHI + frac7 * LN_PHI;
    
    cout << "  n sum: " << n_sum << "\n";
    cout << "  frac sum: " << frac_sum << "\n";
    cout << "  log scaled sum: " << log_scaled_sum << "\n\n";

    // Reconstruct
    double phi_frac_recon = exp(log_scaled_sum);
    double a = fib[(int)n_sum - 1] * phi_frac_recon;
    double b = fib[(int)n_sum] * phi_frac_recon;
    double recovered = a + b * PHI;
    
    cout << "  φ^frac reconstructed: " << phi_frac_recon << "\n";
    cout << "  a = " << a << "\n";
    cout << "  b = " << b << "\n";
    cout << "  Recovered: " << recovered << " (expected: 35)\n";
    cout << "  Match: " << (abs(recovered - 35.0) < 0.01 ? "✅" : "❌") << "\n\n";

    return 0;
}
