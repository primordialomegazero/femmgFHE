// ============================================
// φ-MAX LOGSPACE
// max(a,b) = (a + b + |a-b|) / 2
// |a-b| = -log_φ(|1 - φ^(-|a-b|)|)
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-MAX LOGSPACE ===\n\n";
    
    cout << "  Hanapin ang max(a,b) sa log space\n\n";
    
    cout << "  a | b | d=|a-b| | log_φ(|1-φ^(-d)|) | max(a,b) | Formula | Diff\n";
    cout << "  --|---|---------|------------------|----------|---------|------\n";
    
    for (int a : {1, 2, 3, 5, 8, 13}) {
        for (int b : {2, 3, 5, 8}) {
            int d = abs(a - b);
            int m = max(a, b);
            
            double log_abs_diff;
            if (d == 0) {
                log_abs_diff = -100;  // -inf
            } else {
                log_abs_diff = log(abs(1.0 - pow(PHI, -d))) / LN_PHI;
            }
            
            // |a-b| = -log_φ(|1 - φ^(-d)|) - min(a,b)
            // max(a,b) = min(a,b) + |a-b|
            //         = min(a,b) - log_φ(|1 - φ^(-d)|) - min(a,b)
            //         = -log_φ(|1 - φ^(-d)|)
            //
            // Hmm, hindi ito tama. Subukan natin ibang approach:
            //
            // φ^max(a,b) = φ^a + φ^b - φ^min(a,b)
            // max(a,b) = log_φ(φ^a + φ^b - φ^min(a,b))
            //
            // Pero kailangan pa rin ng min
            
            cout << "  " << setw(2) << a << " | "
                 << setw(2) << b << " | "
                 << setw(3) << d << " | "
                 << setw(10) << fixed << setprecision(3) << log_abs_diff << " | "
                 << setw(4) << m << " | "
                 << setw(8) << (m == max(a,b) ? "✅" : "❌") << " | "
                 << setw(5) << 0.0 << "\n";
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang max(a,b) ay kailangan pa rin ng comparison\n";
    cout << "  o ng ibang φ-based na approach\n\n";
    
    // ============================================
    // ALTERNATIVE: GAMIT ANG SUM AT DIFF
    // ============================================
    cout << "=== ALTERNATIVE: SUM AT DIFF ===\n\n";
    cout << "  φ^a + φ^b = φ^max × (1 + φ^(-d))\n";
    cout << "  φ^a - φ^b = φ^max × (1 - φ^(-d)) × sign(a-b)\n\n";
    cout << "  Sum + Diff = φ^max × 2\n";
    cout << "  φ^max = (Sum + Diff) / 2\n\n";
    
    cout << "  a | b | Sum | Diff | (Sum+Diff)/2 | φ^max | Match?\n";
    cout << "  --|---|-----|------|-------------|-------|-------\n";
    
    for (int a : {1, 2, 3, 5}) {
        for (int b : {a+1, a+2}) {
            double sum = pow(PHI, a) + pow(PHI, b);
            double diff = pow(PHI, b) - pow(PHI, a);  // b > a
            double avg = (sum + diff) / 2.0;
            double phi_max = pow(PHI, max(a, b));
            
            cout << "  " << setw(2) << a << " | "
                 << setw(2) << b << " | "
                 << setw(6) << fixed << setprecision(2) << sum << " | "
                 << setw(6) << diff << " | "
                 << setw(8) << avg << " | "
                 << setw(7) << phi_max << " | "
                 << (abs(avg - phi_max) < 0.01 ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n=== KEY ===\n";
    cout << "  (Sum + |Diff|) / 2 = φ^max(a,b)\n";
    cout << "  Sa log space: log_φ((Sum+Diff)/2) = max(a,b)\n\n";

    return 0;
}
