// ============================================
// φ-STRUCT ADD
// Addition sa log space via φ-structure
// a + bφ = c + dφ (addition sa φ-basis)
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
    cout << "  φ-STRUCT ADD\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST 1: ADDITION SA φ-BASIS
    // ============================================

    cout << "========================================\n";
    cout << "  ADDITION SA φ-BASIS\n";
    cout << "========================================\n\n";

    cout << "  a + bφ + c + dφ = (a+c) + (b+d)φ\n\n";

    // I-decompose ang numbers sa a + bφ form
    cout << "  x | a | b | a + bφ | x | Match?\n";
    cout << "  --|---|---|---------|---|--------\n";

    vector<pair<int, int>> decompositions = {
        {2, 0},   // 2
        {1, 1},   // 3 ≈ 1 + φ = 2.618
        {3, 1},   // 5 ≈ 3 + φ = 4.618
        {5, 2},   // 8 ≈ 5 + 2φ = 8.236
        {8, 3},   // 13 ≈ 8 + 3φ = 12.854
    };

    for (auto [a, b] : decompositions) {
        double val = a + b * PHI;
        cout << "  a=" << setw(2) << a << ", b=" << setw(1) << b
             << " | " << setw(2) << a << " | " << setw(1) << b
             << " | " << fixed << setprecision(4) << val << "\n";
    }

    // ============================================
    // TEST 2: ADDITION SA φ-BASIS — EXACT
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION SA φ-BASIS — EXACT\n";
    cout << "========================================\n\n";

    cout << "  (a₁+b₁φ) + (a₂+b₂φ) = (a₁+a₂) + (b₁+b₂)φ\n\n";

    cout << "  Ex: 2 + 3 = ?\n";
    cout << "  2 = 2 + 0φ\n";
    cout << "  3 = 1 + φ = 2.618 (approx)\n";
    cout << "  2 + 3 = 3 + φ = 4.618\n";
    cout << "  Expected: 5\n\n";

    // ============================================
    // TEST 3: EXACT φ-BASIS DECOMPOSITION
    // ============================================

    cout << "========================================\n";
    cout << "  EXACT φ-BASIS DECOMPOSITION\n";
    cout << "========================================\n\n";

    cout << "  x | a | b | a+bφ | x | Error\n";
    cout << "  --|---|---|-------|---|-------\n";

    for (double x : {2.0, 3.0, 5.0, 7.0, 8.0, 10.0, 11.0, 13.0}) {
        // Hanapin ang best a at b
        int best_a = 0, best_b = 0;
        double min_err = 1e9;
        
        for (int a = -10; a <= 10; a++) {
            for (int b = -10; b <= 10; b++) {
                double val = a + b * PHI;
                double err = abs(val - x);
                if (err < min_err) {
                    min_err = err;
                    best_a = a;
                    best_b = b;
                }
            }
        }
        
        double val = best_a + best_b * PHI;
        cout << "  " << setw(3) << x << " | "
             << setw(2) << best_a << " | "
             << setw(2) << best_b << " | "
             << fixed << setprecision(4) << val << " | "
             << setw(3) << x << " | "
             << setw(5) << min_err << "\n";
    }

    // ============================================
    // TEST 4: ADDITION SA LOG SPACE VIA φ-BASIS
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION SA LOG SPACE VIA φ-BASIS\n";
    cout << "========================================\n\n";

    cout << "  log_φ(a+b) = log_φ((a₁+a₂) + (b₁+b₂)φ)\n";
    cout << "  = log_φ((a₁+a₂) + (b₁+b₂)φ)\n\n";

    cout << "  a | b | a+b | φ-basis | log_φ(a+b) | Match?\n";
    cout << "  --|---|-----|---------|-----------|--------\n";

    for (double a : {2.0, 3.0, 5.0}) {
        for (double b : {2.0, 3.0, 5.0}) {
            double sum = a + b;
            double log_sum = log(sum) / LN_PHI;
            
            // Hanapin ang φ-basis ng sum
            int best_a = 0, best_b = 0;
            double min_err = 1e9;
            
            for (int ia = -10; ia <= 10; ia++) {
                for (int ib = -10; ib <= 10; ib++) {
                    double val = ia + ib * PHI;
                    double err = abs(val - sum);
                    if (err < min_err) {
                        min_err = err;
                        best_a = ia;
                        best_b = ib;
                    }
                }
            }
            
            cout << "  " << setw(2) << a << " | " << setw(1) << b
                 << " | " << setw(3) << sum
                 << " | " << best_a << "+" << best_b << "φ"
                 << " | " << fixed << setprecision(4) << log_sum
                 << " | (err=" << min_err << ")\n";
        }
    }

    return 0;
}
