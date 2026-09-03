// ============================================
// φ-CLOSED FORM SEARCH
// Hanapin ang closed form ng log_φ(1 + φⁿ)
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
    cout << "  φ-CLOSED FORM SEARCH\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST 1: RECURRENCE RELATION
    // ============================================

    cout << "========================================\n";
    cout << "  RECURRENCE RELATION\n";
    cout << "========================================\n\n";

    // I-define: c(n) = log_φ(1 + φⁿ)
    // Hanapin: c(n) + c(m) = ?
    // Hanapin: c(n) × c(m) = ?

    cout << "  c(n) + c(m):\n";
    cout << "  n | m | c(n) | c(m) | c(n)+c(m) | c(n+m)?\n";
    cout << "  --|---|------|------|-----------|--------\n";

    vector<double> c_values;
    for (int n = -3; n <= 3; n++) {
        c_values.push_back(log(1 + pow(PHI, n)) / LN_PHI);
    }

    for (int i = 0; i < 7; i++) {
        for (int j = i; j < 7; j++) {
            int n = i - 3;
            int m = j - 3;
            double sum = c_values[i] + c_values[j];
            double c_nm = (n + m >= -3 && n + m <= 3) ? 
                         c_values[n + m + 3] : NAN;

            cout << "  " << setw(2) << n << " | "
                 << setw(2) << m << " | "
                 << setw(5) << fixed << setprecision(4) << c_values[i] << " | "
                 << setw(5) << c_values[j] << " | "
                 << setw(9) << sum << " | "
                 << setw(8) << (isnan(c_nm) ? "N/A" : to_string(c_nm)) << "\n";
        }
    }

    // ============================================
    // TEST 2: PRODUCT RECURRENCE
    // ============================================

    cout << "\n========================================\n";
    cout << "  PRODUCT RELATION\n";
    cout << "========================================\n\n";

    cout << "  (1 + φⁿ)(1 + φᵐ) = ?\n";
    cout << "  n | m | product | log_φ(product) | c(n)+c(m)\n";
    cout << "  --|---|---------|----------------|----------\n";

    for (int n = -3; n <= 3; n++) {
        for (int m = -3; m <= 3; m++) {
            double prod = (1 + pow(PHI, n)) * (1 + pow(PHI, m));
            double log_prod = log(prod) / LN_PHI;
            double c_sum = c_values[n + 3] + c_values[m + 3];

            cout << "  " << setw(2) << n << " | "
                 << setw(2) << m << " | "
                 << setw(7) << fixed << setprecision(4) << prod << " | "
                 << setw(14) << log_prod << " | "
                 << setw(9) << c_sum << "\n";
        }
    }

    // ============================================
    // TEST 3: EXPANSION SEARCH
    // ============================================

    cout << "\n========================================\n";
    cout << "  EXPANSION SEARCH\n";
    cout << "========================================\n\n";

    // Hanapin: log_φ(1 + φⁿ) = a₀ + a₁φ + a₂φ² + ...
    // Subukan ang 3-term expansion

    cout << "  n | exact | a₀ + a₁φ + a₂φ² | error\n";
    cout << "  --|-------|------------------|-------\n";

    for (int n = -5; n <= 5; n++) {
        double exact = log(1 + pow(PHI, n)) / LN_PHI;
        
        // Hanapin ang best 3-term expansion
        double best_err = 999;
        double best_a0 = 0, best_a1 = 0, best_a2 = 0;
        
        for (int a0 = -10; a0 <= 10; a0++) {
            for (int a1 = -10; a1 <= 10; a1++) {
                for (int a2 = -10; a2 <= 10; a2++) {
                    double val = a0 + a1 * PHI + a2 * PHI * PHI;
                    double err = fabs(val - exact);
                    if (err < best_err) {
                        best_err = err;
                        best_a0 = a0;
                        best_a1 = a1;
                        best_a2 = a2;
                    }
                }
            }
        }

        cout << "  " << setw(2) << n << " | "
             << setw(5) << fixed << setprecision(4) << exact << " | "
             << setw(3) << best_a0 << " + " << setw(3) << best_a1 << "φ + " 
             << setw(3) << best_a2 << "φ² | "
             << setw(6) << best_err << "\n";
    }

    return 0;
}
