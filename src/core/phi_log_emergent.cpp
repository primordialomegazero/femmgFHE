// ============================================
// φ-LOG EMERGENT PROPERTIES
// Hanapin ang pattern ng log_φ(1 + φⁿ)
// para sa addition sa log space
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
    cout << "  φ-LOG EMERGENT PROPERTIES\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST 1: log_φ(1 + φⁿ) PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  log_φ(1 + φⁿ) PATTERN\n";
    cout << "========================================\n\n";

    cout << "  n | φⁿ | 1+φⁿ | log_φ(1+φⁿ) | integer? | difference\n";
    cout << "  --|-----|------|------------|----------|----------\n";

    for (int n = -5; n <= 5; n++) {
        double phi_n = pow(PHI, n);
        double one_plus = 1 + phi_n;
        double log_val = log(one_plus) / LN_PHI;
        double nearest_int = round(log_val);
        double diff = log_val - nearest_int;
        bool is_int = fabs(diff) < 0.001;

        cout << "  " << setw(2) << n << " | "
             << setw(5) << fixed << setprecision(4) << phi_n << " | "
             << setw(4) << one_plus << " | "
             << setw(10) << log_val << " | "
             << setw(8) << (is_int ? "YES" : "no") << " | "
             << setw(8) << diff << "\n";
    }

    // ============================================
    // TEST 2: CORRECTION SEQUENCE
    // ============================================

    cout << "\n========================================\n";
    cout << "  CORRECTION SEQUENCE\n";
    cout << "========================================\n\n";

    cout << "  n | correction | correction - n | pattern?\n";
    cout << "  --|------------|----------------|---------\n";

    for (int n = -5; n <= 5; n++) {
        double phi_n = pow(PHI, n);
        double correction = log(1 + phi_n) / LN_PHI;
        double diff = correction - n;

        cout << "  " << setw(2) << n << " | "
             << setw(10) << fixed << setprecision(6) << correction << " | "
             << setw(12) << diff << " | "
             << setw(8) << (fabs(diff) < 0.001 ? "EXACT" : "approx") << "\n";
    }

    // ============================================
    // TEST 3: φ-DECOMPOSITION NG CORRECTION
    // ============================================

    cout << "\n========================================\n";
    cout << "  φ-DECOMPOSITION NG CORRECTION\n";
    cout << "========================================\n\n";

    cout << "  n | correction | φ-basis (a+bφ) | a | b\n";
    cout << "  --|------------|-----------------|---|---\n";

    for (int n = -5; n <= 5; n++) {
        double phi_n = pow(PHI, n);
        double correction = log(1 + phi_n) / LN_PHI;
        
        // Hanapin ang φ-basis representation:
        // correction = a + bφ
        // Subukan ang integers a, b
        
        double best_a = 0, best_b = 0, best_err = 999;
        for (int a = -10; a <= 10; a++) {
            for (int b = -10; b <= 10; b++) {
                double val = a + b * PHI;
                double err = fabs(val - correction);
                if (err < best_err) {
                    best_err = err;
                    best_a = a;
                    best_b = b;
                }
            }
        }

        cout << "  " << setw(2) << n << " | "
             << setw(10) << fixed << setprecision(6) << correction << " | "
             << setw(15) << best_a << " + " << best_b << "φ | "
             << setw(2) << best_a << " | "
             << setw(2) << best_b << " | err: " << best_err << "\n";
    }

    return 0;
}
