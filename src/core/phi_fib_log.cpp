// ============================================
// φ-FIB LOG
// Fibonacci sa log space
// log_φ(F_n) ≈ n - log_φ(√5)
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
    cout << "  φ-FIB LOG\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double SQRT5 = sqrt(5.0);
    const double LOG_SQRT5 = log(SQRT5) / LN_PHI;

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  log_φ(√5) = " << LOG_SQRT5 << "\n\n";

    // ============================================
    // F_n SA LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  F_n SA LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  n | F_n | log_φ(F_n) | n - log_φ(√5) | Match?\n";
    cout << "  --|-----|-----------|---------------|--------\n";

    for (int n = 2; n <= 15; n++) {
        double log_F = log((double)fib[n]) / LN_PHI;
        double approx = n - LOG_SQRT5;
        bool match = abs(log_F - approx) < 0.01;
        
        cout << "  " << setw(2) << n << " | "
             << setw(4) << fib[n] << " | "
             << setw(9) << fixed << setprecision(4) << log_F << " | "
             << setw(13) << approx << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // MULTIPLICATION SA FIBONACCI LOG SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  MULTIPLICATION SA FIBONACCI LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  log_φ(F_m × F_n) = log_φ(F_m) + log_φ(F_n)\n";
    cout << "  ≈ (m - c) + (n - c) = m + n - 2c\n\n";

    cout << "  m | n | log_φ(F_m×F_n) | m+n-2c | Match?\n";
    cout << "  --|---|---------------|--------|--------\n";

    for (int m = 3; m <= 6; m++) {
        for (int n = 3; n <= 6; n++) {
            long long prod = fib[m] * fib[n];
            double log_prod = log((double)prod) / LN_PHI;
            double approx = m + n - 2 * LOG_SQRT5;
            bool match = abs(log_prod - approx) < 0.1;
            
            cout << "  " << m << " | " << n << " | "
                 << setw(13) << fixed << setprecision(4) << log_prod << " | "
                 << setw(7) << approx << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // ADDITION SA FIBONACCI LOG SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION SA FIBONACCI LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  log_φ(F_m + F_n) = ?\n";
    cout << "  Kung F_m + F_n = F_k:\n";
    cout << "  log_φ(F_k) ≈ k - c\n\n";

    cout << "  m | n | F_m+F_n | k | log_φ(F_m+F_n) | k-c | Match?\n";
    cout << "  --|---|---------|---|---------------|-----|--------\n";

    for (int m = 2; m <= 6; m++) {
        for (int n = 2; n <= 6; n++) {
            long long sum = fib[m] + fib[n];
            double log_sum = log((double)sum) / LN_PHI;
            
            // Hanapin kung sum ay Fibonacci
            int k = -1;
            for (int i = 0; i < fib.size(); i++) {
                if (fib[i] == sum) { k = i; break; }
            }
            
            if (k >= 0) {
                double approx = k - LOG_SQRT5;
                bool match = abs(log_sum - approx) < 0.1;
                
                cout << "  " << m << " | " << n << " | "
                     << setw(6) << sum << " | "
                     << setw(1) << k << " | "
                     << setw(13) << fixed << setprecision(4) << log_sum << " | "
                     << setw(7) << approx << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    return 0;
}
