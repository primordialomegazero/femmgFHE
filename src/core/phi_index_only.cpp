// ============================================
// φ-INDEX ONLY
// I-encode ang index n sa slots
// Ang EvalAdd ay nagbibigay ng n1 + n2
// At ang reconstruction ay mula sa n
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
    cout << "  φ-INDEX ONLY\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 30; i++) {
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    // ============================================
    // INDEX-ONLY ENCODING
    // Slot 0: x
    // Slot 1: n (index)
    // Slot 2: frac (fractional)
    // Slot 3: n + frac = log_φ(x)
    // ============================================

    cout << "  x | n | frac | log_φ(x)\n";
    cout << "  --|---|------|---------\n";

    for (double x : {5.0, 7.0, 35.0, 3.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        double frac = log_phi_x - n;
        
        cout << "  " << setw(3) << x << " | "
             << setw(1) << n << " | "
             << setw(4) << fixed << setprecision(4) << frac << " | "
             << setw(7) << log_phi_x << "\n";
    }

    // ============================================
    // ADDITION TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION TEST\n";
    cout << "========================================\n\n";

    // 5: n=3, frac=0.3446
    // 7: n=4, frac=0.0438
    
    double n5 = 3, frac5 = 0.3446;
    double n7 = 4, frac7 = 0.0438;
    
    double n_sum = n5 + n7;
    double frac_sum = frac5 + frac7;
    
    cout << "  n(5) + n(7) = " << n_sum << "\n";
    cout << "  frac(5) + frac(7) = " << frac_sum << "\n";
    cout << "  log_φ(35) = " << (n_sum + frac_sum) << "\n\n";

    // Reconstruct 35 mula sa n at frac
    int n_recon = (int)n_sum;
    double phi_frac = pow(PHI, frac_sum);
    double F_n = fib[n_recon];
    double F_nm1 = fib[n_recon - 1];
    double phi_n = F_nm1 + F_n * PHI;  // φ^n = F_{n-1} + F_n × φ
    double x_recon = phi_n * phi_frac;
    
    cout << "  Reconstructed: " << x_recon << " (expected: 35)\n";
    cout << "  Match: " << (abs(x_recon - 35.0) < 0.01 ? "✅" : "❌") << "\n\n";

    return 0;
}
