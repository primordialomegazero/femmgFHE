// ============================================
// φ-SELF-REFERENTIAL SPACE V2
// Ang φ² = φ + 1 bilang natural na space
// para sa lahat ng operations
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
    cout << "  φ-SELF-REFERENTIAL SPACE V2\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // SELF-REFERENTIAL ENCODING
    // x = a + bφ kung saan φ² = φ + 1
    // ============================================

    cout << "========================================\n";
    cout << "  SELF-REFERENTIAL ENCODING\n";
    cout << "========================================\n\n";

    cout << "  x = a + bφ\n";
    cout << "  φ² = φ + 1\n";
    cout << "  φ³ = 2φ + 1\n";
    cout << "  φ⁴ = 3φ + 2\n\n";

    cout << "  x | a | b | a + bφ | Match?\n";
    cout << "  --|---|---|---------|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 35.0}) {
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
        
        cout << "  " << setw(3) << x << " | "
             << setw(2) << best_a << " | "
             << setw(2) << best_b << " | "
             << fixed << setprecision(4) << (best_a + best_b * PHI) << " | "
             << (min_err < 0.01 ? "✅" : "❌") << "\n";
    }

    // ============================================
    // ADDITION SA SELF-REFERENTIAL SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION SA SELF-REF SPACE\n";
    cout << "========================================\n\n";

    cout << "  (a₁ + b₁φ) + (a₂ + b₂φ) = (a₁+a₂) + (b₁+b₂)φ\n\n";

    cout << "  a+b | Self-ref result | Normal sum | Match?\n";
    cout << "  ----|----------------|-----------|--------\n";

    for (double a : {2.0, 3.0, 5.0}) {
        for (double b : {2.0, 3.0, 5.0}) {
            // Decompose a at b
            int a1 = 0, b1 = 0, a2 = 0, b2 = 0;
            double min1 = 1e9, min2 = 1e9;
            
            for (int ia = -10; ia <= 10; ia++) {
                for (int ib = -10; ib <= 10; ib++) {
                    double v1 = ia + ib * PHI;
                    if (abs(v1 - a) < min1) { min1 = abs(v1 - a); a1 = ia; b1 = ib; }
                    if (abs(v1 - b) < min2) { min2 = abs(v1 - b); a2 = ia; b2 = ib; }
                }
            }
            
            double self_ref_sum = (a1 + a2) + (b1 + b2) * PHI;
            double normal_sum = a + b;
            
            cout << "  " << setw(2) << a << "+" << setw(1) << b << " | "
                 << fixed << setprecision(4) << self_ref_sum << " | "
                 << setw(9) << normal_sum << " | "
                 << (abs(self_ref_sum - normal_sum) < 0.01 ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // MULTIPLICATION SA SELF-REFERENTIAL SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  MULTIPLICATION SA SELF-REF SPACE\n";
    cout << "========================================\n\n";

    cout << "  (a₁ + b₁φ)(a₂ + b₂φ) = (a₁a₂ + b₁b₂) + (a₁b₂ + b₁a₂ + b₁b₂)φ\n\n";

    cout << "  a×b | Self-ref result | Normal product | Match?\n";
    cout << "  ----|----------------|---------------|--------\n";

    for (double a : {2.0, 3.0, 5.0}) {
        for (double b : {2.0, 3.0, 5.0}) {
            int a1 = 0, b1 = 0, a2 = 0, b2 = 0;
            double min1 = 1e9, min2 = 1e9;
            
            for (int ia = -10; ia <= 10; ia++) {
                for (int ib = -10; ib <= 10; ib++) {
                    double v1 = ia + ib * PHI;
                    if (abs(v1 - a) < min1) { min1 = abs(v1 - a); a1 = ia; b1 = ib; }
                    if (abs(v1 - b) < min2) { min2 = abs(v1 - b); a2 = ia; b2 = ib; }
                }
            }
            
            double self_ref_prod = (a1*a2 + b1*b2) + (a1*b2 + b1*a2 + b1*b2) * PHI;
            double normal_prod = a * b;
            
            cout << "  " << setw(2) << a << "×" << setw(1) << b << " | "
                 << fixed << setprecision(4) << self_ref_prod << " | "
                 << setw(13) << normal_prod << " | "
                 << (abs(self_ref_prod - normal_prod) < 0.1 ? "✅" : "❌") << "\n";
        }
    }

    return 0;
}
