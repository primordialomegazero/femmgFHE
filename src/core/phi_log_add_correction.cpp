// ============================================
// φ-LOG ADD CORRECTION
// I-encode ang correction log_φ(1 + b/a)
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
    cout << "  φ-LOG ADD CORRECTION\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  a | b | b/a | 1+b/a | log_φ(1+b/a) | log_φ(a+b)\n";
    cout << "  --|---|-----|-------|-------------|-----------\n";

    for (double a : {2.0, 3.0, 5.0, 8.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0}) {
            double ratio = b / a;
            double one_plus = 1 + ratio;
            double correction = log(one_plus) / LN_PHI;
            double log_a = log(a) / LN_PHI;
            double log_add = log_a + correction;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(1) << b << " | "
                 << setw(5) << fixed << setprecision(3) << ratio << " | "
                 << setw(5) << one_plus << " | "
                 << setw(11) << correction << " | "
                 << setw(9) << log_add << "\n";
        }
    }

    // ============================================
    // CORRECTION PATTERN
    // ============================================

    cout << "\n========================================\n";
    cout << "  CORRECTION PATTERN\n";
    cout << "========================================\n\n";

    cout << "  Ang correction ay log_φ(1 + b/a):\n";
    cout << "  - Bounded sa [0, 1] para sa b/a ≤ φ-1\n";
    cout << "  - May natural na periodicity\n";
    cout << "  - Pwedeng i-pre-compute at i-encode\n\n";

    // Pre-computed correction table
    cout << "  CORRECTION TABLE (b/a):\n";
    for (double ratio : {0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 2.0}) {
        double correction = log(1 + ratio) / LN_PHI;
        cout << "  b/a = " << setw(4) << ratio << " → correction = " 
             << fixed << setprecision(4) << correction << "\n";
    }

    return 0;
}
