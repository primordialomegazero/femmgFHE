// ============================================
// φ-CORRECTION GENERAL
// I-verify kung ang correction ay
// palaging φ-power
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
    cout << "  φ-CORRECTION GENERAL\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST: CORRECTION = φ⁻ᵏ PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  CORRECTION = φ⁻ᵏ PATTERN\n";
    cout << "========================================\n\n";

    cout << "  a+b | sum | φ-decomp | exact | correction | k (φ⁻ᵏ)\n";
    cout << "  ----|-----|----------|-------|-----------|--------\n";

    vector<int> sums = {3,4,5,6,7,8,9,10,11,13,14,15,16,18,21,24,26,29,34,42};

    for (int sum : sums) {
        // Hanapin ang pinakamalapit na φ^m ± φ^n
        double best_val = 0;
        double min_err = 1e9;
        string best_form = "";

        for (int m = 0; m <= 10; m++) {
            for (int n = 0; n <= 10; n++) {
                double phi_m = pow(PHI, m);
                double phi_n = pow(PHI, n);
                double sum_phi = phi_m + phi_n;
                double diff_phi = phi_m - phi_n;

                if (abs(sum_phi - sum) < min_err) {
                    min_err = abs(sum_phi - sum);
                    best_form = "φ^" + to_string(m) + "+φ^" + to_string(n);
                }
                if (abs(diff_phi - sum) < min_err) {
                    min_err = abs(diff_phi - sum);
                    best_form = "φ^" + to_string(m) + "-φ^" + to_string(n);
                }
            }
        }

        // Hanapin ang k sa φ⁻ᵏ
        double correction = min_err;
        double k_val = -log(correction) / log(PHI);

        cout << "  " << setw(2) << sum << " | "
             << setw(4) << sum << " | "
             << setw(8) << best_form << " | "
             << setw(5) << fixed << setprecision(4) << sum << " | "
             << setw(9) << correction << " | "
             << setw(6) << k_val << "\n";
    }

    // ============================================
    // TEST: CORRECTION SIGN PATTERN
    // ============================================

    cout << "\n========================================\n";
    cout << "  CORRECTION SIGN PATTERN\n";
    cout << "========================================\n\n";

    cout << "  sum | Fibonacci? | Lucas? | correction sign\n";
    cout << "  ----|-----------|--------|---------------\n";

    for (int sum : sums) {
        bool is_fib = false;
        int fib_n = 0;
        vector<int> fibs = {0,1,1,2,3,5,8,13,21,34};
        for (int i = 0; i < fibs.size(); i++) {
            if (fibs[i] == sum) {
                is_fib = true;
                fib_n = i;
                break;
            }
        }

        bool is_lucas = false;
        int lucas_n = 0;
        vector<int> lucas = {2,1,3,4,7,11,18,29,47};
        for (int i = 0; i < lucas.size(); i++) {
            if (lucas[i] == sum) {
                is_lucas = true;
                lucas_n = i;
                break;
            }
        }

        // Find correction sign
        double correction = 0;
        for (int m = 0; m <= 10; m++) {
            for (int n = 0; n <= 10; n++) {
                double phi_m = pow(PHI, m);
                double phi_n = pow(PHI, n);
                if (abs(phi_m + phi_n - sum) < abs(correction)) {
                    correction = phi_m + phi_n - sum;
                }
                if (abs(phi_m - phi_n - sum) < abs(correction)) {
                    correction = phi_m - phi_n - sum;
                }
            }
        }

        cout << "  " << setw(2) << sum << " | "
             << setw(9) << (is_fib ? "F_" + to_string(fib_n) : "no") << " | "
             << setw(6) << (is_lucas ? "L_" + to_string(lucas_n) : "no") << " | "
             << setw(9) << (correction > 0 ? "+" : "-") << "\n";
    }

    return 0;
}
