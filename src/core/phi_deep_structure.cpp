// ============================================
// φ-DEEP STRUCTURE
// Hanapin ang mas malalim na φ-structure
// kung saan addition at multiplication
// ay parehong EvalAdd lang
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
    cout << "  φ-DEEP STRUCTURE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST 1: LOG-LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  LOG-LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  Subok: log_φ(log_φ(x))\n";
    cout << "  x | log_φ(x) | log_φ(log_φ(x))\n";
    cout << "  --|-----------|----------------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0}) {
        double log_x = log(x) / LN_PHI;
        double log_log = log(log_x) / LN_PHI;
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_x << " | "
             << setw(14) << log_log << "\n";
    }

    // ============================================
    // TEST 2: ADDITION SA LOG-LOG SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION SA LOG-LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  log_φ(log_φ(a)) + log_φ(log_φ(b)) = ?\n";
    cout << "  a | b | loglog(a) | loglog(b) | Sum\n";
    cout << "  --|---|-----------|-----------|-----\n";

    for (double a : {2.0, 3.0, 5.0}) {
        for (double b : {2.0, 3.0, 5.0}) {
            double log_log_a = log(log(a)/LN_PHI) / LN_PHI;
            double log_log_b = log(log(b)/LN_PHI) / LN_PHI;
            double sum = log_log_a + log_log_b;
            
            cout << "  " << setw(2) << a << " | " << setw(1) << b
                 << " | " << fixed << setprecision(4) << log_log_a
                 << " | " << log_log_b
                 << " | " << sum << "\n";
        }
    }

    // ============================================
    // TEST 3: EXPONENTIAL SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  EXPONENTIAL SPACE\n";
    cout << "========================================\n\n";

    cout << "  Subok: φ^x (exponential)\n";
    cout << "  x | φ^x | φ^x + φ^y\n";
    cout << "  --|-----|-----------\n";

    for (double x : {1.0, 2.0, 3.0}) {
        double phi_x = pow(PHI, x);
        cout << "  " << setw(2) << x << " | "
             << setw(7) << fixed << setprecision(4) << phi_x << "\n";
    }

    // ============================================
    // TEST 4: SOFTPLUS SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  SOFTPLUS SPACE\n";
    cout << "========================================\n\n";

    cout << "  softplus(x) = log(1 + exp(x))\n";
    cout << "  softplus(x) + softplus(y) = ?\n\n";

    auto softplus = [&](double x) {
        return log(1 + exp(x)) / LN_PHI;
    };

    cout << "  x | softplus(x)\n";
    cout << "  --|------------\n";
    for (double x : {-2.0, -1.0, 0.0, 1.0, 2.0}) {
        cout << "  " << setw(3) << x << " | "
             << fixed << setprecision(4) << softplus(x) << "\n";
    }

    // ============================================
    // TEST 5: φ-POWER SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  φ-POWER SPACE\n";
    cout << "========================================\n\n";

    cout << "  x = φ^n → n = log_φ(x)\n";
    cout << "  Sa φ-power space:\n";
    cout << "  Addition: φ^m + φ^n = ?\n";
    cout << "  Multiplication: φ^m × φ^n = φ^(m+n)\n\n";

    cout << "  m | n | φ^m + φ^n | log_φ(φ^m + φ^n)\n";
    cout << "  --|---|-----------|--------------------\n";

    for (int m = 0; m <= 5; m++) {
        for (int n = 0; n <= 5; n++) {
            double phi_m = pow(PHI, m);
            double phi_n = pow(PHI, n);
            double sum = phi_m + phi_n;
            double log_sum = log(sum) / LN_PHI;
            
            cout << "  " << m << " | " << n
                 << " | " << fixed << setprecision(4) << sum
                 << " | " << log_sum << "\n";
        }
    }

    // ============================================
    // KEY PATTERN
    // ============================================

    cout << "\n========================================\n";
    cout << "  KEY PATTERN\n";
    cout << "========================================\n\n";

    cout << "  φ^m + φ^(m-1) = φ^(m+1)\n";
    cout << "  m | φ^m + φ^(m-1) | φ^(m+1) | Match?\n";
    cout << "  --|---------------|---------|--------\n";

    for (int m = 1; m <= 8; m++) {
        double sum = pow(PHI, m) + pow(PHI, m-1);
        double next = pow(PHI, m+1);
        bool match = abs(sum - next) < 0.001;
        
        cout << "  " << m << " | "
             << setw(13) << fixed << setprecision(4) << sum << " | "
             << setw(7) << next << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    return 0;
}
