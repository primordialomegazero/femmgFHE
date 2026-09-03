// ============================================
// φ-ZERO ADD
// Addition sa φ-power space via zero-reference
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
    cout << "  φ-ZERO ADD\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  Zero-referenced addition:\n";
    cout << "  (φ^m - 1) + (φ^n - 1) = φ^m + φ^n - 2\n";
    cout << "  = (φ^m + φ^n) - 2\n\n";

    cout << "  m | n | φ^m + φ^n - 2 | x+y-2 | Match?\n";
    cout << "  --|---|---------------|-------|--------\n";

    for (double m : {2.0, 3.0, 4.0}) {
        for (double n : {2.0, 3.0, 4.0}) {
            double phi_m = pow(PHI, m);
            double phi_n = pow(PHI, n);
            double sum = phi_m + phi_n - 2;
            
            double x = phi_m;
            double y = phi_n;
            double normal_sum = x + y - 2;
            
            bool match = abs(sum - normal_sum) < 0.001;
            
            cout << "  " << setw(2) << m << " | " << setw(1) << n
                 << " | " << fixed << setprecision(4) << sum
                 << " | " << fixed << setprecision(4) << normal_sum
                 << " | " << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // ZERO-REFERENCED INDEX
    // ============================================

    cout << "\n========================================\n";
    cout << "  ZERO-REFERENCED INDEX\n";
    cout << "========================================\n\n";

    cout << "  n - 0 = n\n";
    cout << "  (m - 0) + (n - 0) = m + n\n\n";

    cout << "  Ito ay natural na index addition para sa multiplication!\n\n";

    cout << "  m | n | m + n | φ^(m+n)\n";
    cout << "  --|---|-------|--------\n";

    for (int m : {2, 3, 4}) {
        for (int n : {2, 3, 4}) {
            double phi_mn = pow(PHI, m + n);
            cout << "  " << m << " | " << n << " | " << (m+n)
                 << " | " << fixed << setprecision(4) << phi_mn << "\n";
        }
    }

    return 0;
}
