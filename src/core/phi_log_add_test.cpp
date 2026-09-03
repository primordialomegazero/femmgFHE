// ============================================
// φ-LOG ADD — ADDITION SA LOG SPACE
// Hanapin kung may paraan para mag-add
// sa log space nang walang normal space
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
    cout << "  φ-LOG ADD — ADDITION SA LOG SPACE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST: log_φ(a) + log_φ(b) = log_φ(a×b)
    // Hanapin: log_φ(a+b) = ?
    // ============================================

    cout << "========================================\n";
    cout << "  ADDITION SA LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  a | b | log_φ(a) | log_φ(b) | log_φ(a+b) | log_φ(a×b)\n";
    cout << "  --|---|-----------|-----------|------------|----------\n";

    for (double a : {2.0, 3.0, 5.0, 8.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0}) {
            double log_a = log(a) / LN_PHI;
            double log_b = log(b) / LN_PHI;
            double log_add = log(a + b) / LN_PHI;
            double log_mult = log_a + log_b;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(1) << b << " | "
                 << setw(9) << fixed << setprecision(4) << log_a << " | "
                 << setw(9) << log_b << " | "
                 << setw(10) << log_add << " | "
                 << setw(8) << log_mult << "\n";
        }
    }

    // ============================================
    // TEST: PATTERN HANAPIN
    // ============================================

    cout << "\n========================================\n";
    cout << "  PATTERN HANAPIN\n";
    cout << "========================================\n\n";

    cout << "  log_φ(a+b) vs log_φ(a) at log_φ(b)\n";
    cout << "  a | b | log_φ(a+b) | log_φ(a)+log_φ(b) | Diff\n";
    cout << "  --|---|------------|--------------------|------\n";

    for (double a : {2.0, 3.0, 5.0, 8.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0}) {
            double log_add = log(a + b) / LN_PHI;
            double log_sum = log(a) / LN_PHI + log(b) / LN_PHI;
            double diff = log_add - log_sum;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(1) << b << " | "
                 << setw(10) << fixed << setprecision(4) << log_add << " | "
                 << setw(18) << log_sum << " | "
                 << setw(5) << diff << "\n";
        }
    }

    // ============================================
    // TEST: LOG-SPACE ADDITION FORMULA
    // ============================================

    cout << "\n========================================\n";
    cout << "  LOG-SPACE ADDITION FORMULA\n";
    cout << "========================================\n\n";

    cout << "  a + b = a × (1 + b/a)\n";
    cout << "  log_φ(a+b) = log_φ(a) + log_φ(1 + b/a)\n\n";

    for (double a : {2.0, 3.0, 5.0, 8.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0}) {
            double log_add = log(a + b) / LN_PHI;
            double log_a = log(a) / LN_PHI;
            double correction = log_add - log_a;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(1) << b << " | "
                 << "correction: " << fixed << setprecision(4) << correction << "\n";
        }
    }

    return 0;
}
