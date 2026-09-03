// ============================================
// φ-SELF-REFERENTIAL STRUCTURE
// I-decompose ang values sa a + bφ form
// para makita ang natural na conversion
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
    cout << "  φ-SELF-REFERENTIAL STRUCTURE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // DECOMPOSE x SA a + bφ FORM
    // ============================================

    cout << "========================================\n";
    cout << "  DECOMPOSE x → a + bφ\n";
    cout << "========================================\n\n";

    cout << "  x | a | b | a + bφ | Match?\n";
    cout << "  --|---|---|---------|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 35.0, 55.0}) {
        // Hanapin ang best a at b
        int best_a = 0, best_b = 0;
        double min_err = 1e9;
        
        for (int a = -20; a <= 20; a++) {
            for (int b = -20; b <= 20; b++) {
                double val = a + b * PHI;
                double err = abs(val - x);
                if (err < min_err) {
                    min_err = err;
                    best_a = a;
                    best_b = b;
                }
            }
        }
        
        double recovered = best_a + best_b * PHI;
        bool match = abs(recovered - x) < 0.01;
        
        cout << "  " << setw(3) << x << " | "
             << setw(3) << best_a << " | "
             << setw(3) << best_b << " | "
             << setw(8) << fixed << setprecision(4) << recovered << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST: ADDITION SA a + bφ SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION SA a + bφ SPACE\n";
    cout << "========================================\n\n";

    cout << "  (5 × 7) + 3 = 38\n";
    cout << "  Sa a + bφ space:\n\n";

    // 5 = ?
    // 5 ≈ 1 + 2φ = 1 + 3.236 = 4.236 (malapit)
    // 5 ≈ 3 + 1φ = 3 + 1.618 = 4.618 (mas malapit)
    // 5 ≈ 2 + 2φ = 2 + 3.236 = 5.236 (mas malayo)
    
    // Subok: 5 = 3 + φ
    double a5 = 3, b5 = 1;
    double val5 = a5 + b5 * PHI;
    
    // 7 = 4 + 2φ = 4 + 3.236 = 7.236 (malapit)
    double a7 = 4, b7 = 2;
    double val7 = a7 + b7 * PHI;
    
    // 35 = ?
    // 35 ≈ 21 + 9φ = 21 + 14.562 = 35.562
    double a35 = 21, b35 = 9;
    double val35 = a35 + b35 * PHI;
    
    cout << "  5 ≈ " << a5 << " + " << b5 << "φ = " << val5 << "\n";
    cout << "  7 ≈ " << a7 << " + " << b7 << "φ = " << val7 << "\n";
    cout << "  35 ≈ " << a35 << " + " << b35 << "φ = " << val35 << "\n\n";

    // ============================================
    // SELF-REFERENTIAL CONVERSION
    // ============================================

    cout << "========================================\n";
    cout << "  SELF-REFERENTIAL CONVERSION\n";
    cout << "========================================\n\n";

    cout << "  φ^n sa a + bφ form:\n";
    cout << "  n | a | b | a + bφ\n";
    cout << "  --|---|---|--------\n";

    for (int n = 0; n <= 10; n++) {
        double phi_n = pow(PHI, n);
        int a = 0, b = 0;
        double min_err = 1e9;
        
        for (int ia = -50; ia <= 50; ia++) {
            for (int ib = -50; ib <= 50; ib++) {
                double val = ia + ib * PHI;
                double err = abs(val - phi_n);
                if (err < min_err) {
                    min_err = err;
                    a = ia;
                    b = ib;
                }
            }
        }
        
        cout << "  " << setw(2) << n << " | "
             << setw(3) << a << " | "
             << setw(3) << b << " | "
             << setw(8) << fixed << setprecision(4) << phi_n << "\n";
    }

    return 0;
}
