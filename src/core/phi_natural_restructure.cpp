// ============================================
// φ-NATURAL RESTRUCTURE
// Hanapin ang natural na φ-property na
// nagre-reconstruct ng value mula sa log space
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
    cout << "  φ-NATURAL RESTRUCTURE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // TEST 1: φ^n = F_n × φ + F_{n-1}
    // Ito ay natural na restructure!
    // ============================================

    cout << "========================================\n";
    cout << "  φ^n = F_n×φ + F_{n-1}\n";
    cout << "========================================\n\n";

    cout << "  Ito ay natural na restructure:\n";
    cout << "  φ^n → (F_{n-1}, F_n) → F_{n-1} + F_n×φ\n\n";

    cout << "  n | φ^n | F_{n-1} | F_n | F_{n-1}+F_n×φ\n";
    cout << "  --|-----|---------|-----|---------------\n";

    for (int n = 1; n <= 10; n++) {
        double phi_n = pow(PHI, n);
        double restructured = fib[n-1] + fib[n] * PHI;
        
        cout << "  " << setw(2) << n << " | "
             << setw(7) << fixed << setprecision(4) << phi_n << " | "
             << setw(7) << fib[n-1] << " | "
             << setw(3) << fib[n] << " | "
             << setw(13) << restructured << "\n";
    }

    // ============================================
    // TEST 2: LOG SPACE → NORMAL SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  LOG SPACE → NORMAL SPACE\n";
    cout << "========================================\n\n";

    cout << "  log_φ(x) = n + frac\n";
    cout << "  x = φ^n × φ^frac\n";
    cout << "  = (F_{n-1} + F_n×φ) × φ^frac\n\n";

    cout << "  x | log_φ(x) | n | frac | F_{n-1}+F_nφ | φ^frac | Restructured\n";
    cout << "  --|-----------|----|------|-------------|--------|------------\n";

    for (double x : {5.0, 7.0, 35.0}) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_n = fib[n-1] + fib[n] * PHI;
        double phi_frac = pow(PHI, frac);
        double restructured = phi_n * phi_frac;
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(2) << n << " | "
             << setw(5) << frac << " | "
             << setw(11) << phi_n << " | "
             << setw(7) << phi_frac << " | "
             << setw(10) << restructured << "\n";
    }

    // ============================================
    // TEST 3: NATURAL RESTRUCTURE VIA φ-RECURRENCE
    // ============================================

    cout << "\n========================================\n";
    cout << "  NATURAL RESTRUCTURE VIA φ-RECURRENCE\n";
    cout << "========================================\n\n";

    cout << "  φ^(n+1) = φ^n × φ = φ^n + φ^(n-1)\n";
    cout << "  Ito ay natural na recurrence!\n\n";

    cout << "  n | φ^n + φ^(n-1) | φ^(n+1) | Match?\n";
    cout << "  --|---------------|---------|--------\n";

    for (int n = 1; n <= 8; n++) {
        double sum = pow(PHI, n) + pow(PHI, n-1);
        double next = pow(PHI, n+1);
        
        cout << "  " << n << " | "
             << setw(13) << fixed << setprecision(4) << sum << " | "
             << setw(7) << next << " | "
             << (abs(sum - next) < 0.001 ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST 4: FRACTIONAL RESTRUCTURE
    // ============================================

    cout << "\n========================================\n";
    cout << "  FRACTIONAL RESTRUCTURE\n";
    cout << "========================================\n\n";

    cout << "  φ^frac ay may natural na periodicity:\n";
    cout << "  frac | φ^frac | φ^frac - 1\n";
    cout << "  -----|--------|----------\n";

    for (double frac : {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9}) {
        double phi_frac = pow(PHI, frac);
        cout << "  " << setw(5) << frac << " | "
             << setw(7) << fixed << setprecision(4) << phi_frac << " | "
             << setw(8) << (phi_frac - 1.0) << "\n";
    }

    // ============================================
    // KEY INSIGHT
    // ============================================

    cout << "\n========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n\n";

    cout << "  Ang natural na restructure ay:\n";
    cout << "  φ^log_φ(x) = x\n";
    cout << "  = (F_{n-1} + F_n×φ) × φ^frac\n\n";

    cout << "  Ang F_{n-1} at F_n ay pwedeng i-pre-compute\n";
    cout << "  at i-encode sa slots.\n\n";

    cout << "  Ang φ^frac ay may natural na periodicity\n";
    cout << "  at pwedeng i-encode sa slots din.\n\n";

    return 0;
}
