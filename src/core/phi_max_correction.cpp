// ============================================
// φ-MAX CORRECTION
// Correction table para sa φ-addition
// log_φ(φ^a + φ^b) = max(a,b) + correction(|a-b|)
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-MAX CORRECTION ===\n\n";
    
    vector<double> correction(20, 0.0);
    
    cout << "  d | correction(d) | φ^(-d) | Match?\n";
    cout << "  --|---------------|---------|-------\n";
    
    for (int d = 0; d <= 10; d++) {
        correction[d] = log(1.0 + pow(PHI, -d)) / LN_PHI;
        
        cout << "  " << setw(2) << d << " | "
             << setw(10) << fixed << setprecision(4) << correction[d] << " | "
             << setw(8) << pow(PHI, -d) << " | "
             << (abs(correction[d] - pow(PHI, -d)) < 0.1 ? "✅" : "  ") << "\n";
    }
    
    cout << "\n=== TEST: φ^a + φ^b ===\n\n";
    cout << "  a | b | max(a,b) | exact log | max+corr | Diff\n";
    cout << "  --|---|----------|-----------|----------|------\n";
    
    for (int a : {1, 2, 3, 5, 8}) {
        for (int b : {a, a+1, a+2, a+3}) {
            double exact_log = log(pow(PHI, a) + pow(PHI, b)) / LN_PHI;
            int m = max(a, b);
            int d = abs(a - b);
            double approx_log = m + correction[d];
            double diff = exact_log - approx_log;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(2) << b << " | "
                 << setw(4) << m << " | "
                 << setw(8) << exact_log << " | "
                 << setw(8) << approx_log << " | "
                 << setw(7) << diff << "\n";
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Kung ang diff ay maliit,\n";
    cout << "  ang correction table ay sapat na\n";
    cout << "  para sa φ-addition sa log space\n\n";

    return 0;
}
