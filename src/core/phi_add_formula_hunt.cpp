// ============================================
// φ-ADD FORMULA HUNT
// Subukan LAHAT ng possible na formula
// para sa addition sa φ-power space
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-ADD FORMULA HUNT\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double SQRT5 = sqrt(5.0);

    // ============================================
    // SUBOK 1: φ^a + φ^b = φ^c + φ^d
    // ============================================

    cout << "========================================\n";
    cout << "  SUBOK 1: φ^a + φ^b = φ^c + φ^d\n";
    cout << "========================================\n\n";

    cout << "  a=2, b=3: φ² + φ³ = 2.618 + 4.236 = 6.854 = φ⁴\n";
    cout << "  Kaya: φ² + φ³ = φ⁴\n\n";

    cout << "  a=3, b=4: φ³ + φ⁴ = 4.236 + 6.854 = 11.09 = φ⁵\n";
    cout << "  Kaya: φ³ + φ⁴ = φ⁵\n\n";

    cout << "  PATTERN: φ^n + φ^(n+1) = φ^(n+2)!\n";
    cout << "  Ito ay EXACT — dahil sa φ² = φ+1\n\n";

    // ============================================
    // SUBOK 2: φ^a + φ^b para sa arbitrary a,b
    // ============================================

    cout << "========================================\n";
    cout << "  SUBOK 2: ARBITRARY a,b\n";
    cout << "========================================\n\n";

    cout << "  φ^a + φ^b = ?\n";
    cout << "  a | b | φ^a + φ^b | φ^(a+b) | φ^(a+1) | φ^(b+1)\n";
    cout << "  --|---|-----------|---------|---------|--------\n";

    for (double a : {1.0, 2.0, 3.0}) {
        for (double b : {a-1, a, a+1}) {
            double sum = pow(PHI, a) + pow(PHI, b);
            double phi_ab = pow(PHI, a+b);
            double phi_a1 = pow(PHI, a+1);
            double phi_b1 = pow(PHI, b+1);
            
            cout << "  " << setw(2) << a << " | " << setw(2) << b << " | "
                 << setw(9) << fixed << setprecision(4) << sum << " | "
                 << setw(7) << phi_ab << " | "
                 << setw(7) << phi_a1 << " | "
                 << setw(7) << phi_b1 << "\n";
        }
    }

    // ============================================
    // SUBOK 3: SOFTPLUS-LIKE
    // ============================================

    cout << "\n========================================\n";
    cout << "  SUBOK 3: SOFTPLUS-LIKE\n";
    cout << "========================================\n\n";

    cout << "  softplus(x) = log(1 + e^x)\n";
    cout << "  Sa φ: softplus_φ(x) = log_φ(1 + φ^x)\n\n";

    cout << "  softplus_φ(a) + softplus_φ(b) = ?\n";
    cout << "  a | b | sp(a) | sp(b) | sp(a)+sp(b)\n";
    cout << "  --|---|-------|-------|-----------\n";

    for (double a : {0.0, 0.5, 1.0}) {
        for (double b : {0.0, 0.5, 1.0}) {
            double sp_a = log(1 + pow(PHI, a)) / LN_PHI;
            double sp_b = log(1 + pow(PHI, b)) / LN_PHI;
            
            cout << "  " << setw(2) << a << " | " << setw(2) << b << " | "
                 << setw(5) << fixed << setprecision(4) << sp_a << " | "
                 << setw(5) << sp_b << " | "
                 << setw(9) << (sp_a + sp_b) << "\n";
        }
    }

    // ============================================
    // SUBOK 4: FIBONACCI IDENTITY
    // ============================================

    cout << "\n========================================\n";
    cout << "  SUBOK 4: FIBONACCI IDENTITY\n";
    cout << "========================================\n\n";

    cout << "  F_{a+b} = F_a × L_b + (-1)^b × F_{a-b}\n";
    cout << "  L_{a+b} = L_a × L_b + (-1)^b × L_{a-b}\n\n";

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) fib.push_back(fib[i-1] + fib[i-2]);

    cout << "  a | b | F_{a+b} | Formula | Match?\n";
    cout << "  --|---|---------|---------|-------\n";

    for (int a = 2; a <= 5; a++) {
        for (int b = 2; b <= 5; b++) {
            long long expected = fib[a+b];
            long long formula = fib[a] * (pow(PHI,b) + pow(-1/PHI,b)) + pow(-1,b) * fib[a-b];
            
            cout << "  " << a << " | " << b << " | "
                 << setw(7) << expected << " | "
                 << setw(7) << formula << " | "
                 << (expected == formula ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // SUBOK 5: PHI-POWER DECOMPOSITION
    // ============================================

    cout << "\n========================================\n";
    cout << "  SUBOK 5: PHI-POWER DECOMPOSITION\n";
    cout << "========================================\n\n";

    cout << "  φ^a + φ^b = φ^min(a,b) × (1 + φ^|a-b|)\n\n";

    for (double a : {2.0, 3.0, 4.0}) {
        for (double b : {2.0, 3.0, 4.0}) {
            double min_ab = min(a, b);
            double diff = abs(a - b);
            double sum = pow(PHI, a) + pow(PHI, b);
            double formula = pow(PHI, min_ab) * (1 + pow(PHI, diff));
            
            cout << "  a=" << a << ", b=" << b << ": "
                 << "φ^" << min_ab << " × (1 + φ^" << diff << ") = "
                 << fixed << setprecision(4) << formula
                 << " | Sum = " << sum
                 << " | " << (abs(formula - sum) < 0.001 ? "✅" : "❌") << "\n";
        }
    }

    return 0;
}
