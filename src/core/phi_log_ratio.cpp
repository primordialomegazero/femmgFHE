// ============================================
// φ-LOG RATIO SPACE
// F = r × φ^k, log_φ(r) ∈ [0, 1)
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-LOG RATIO SPACE ===\n\n";
    
    cout << "  F | k | r | log_φ(r) | log_φ(F)\n";
    cout << "  --|---|----|----------|---------\n";
    
    for (double F : {1.0, 2.0, 3.0, 5.0, 7.0, 8.0, 13.0, 21.0, 34.0, 55.0, 100.0}) {
        double k = floor(log(F) / LN_PHI);
        double r = F / pow(PHI, k);
        double log_r = log(r) / LN_PHI;
        double log_F = log(F) / LN_PHI;
        
        cout << "  " << setw(5) << F << " | "
             << setw(2) << (int)k << " | "
             << setw(5) << fixed << setprecision(3) << r << " | "
             << setw(6) << log_r << " | "
             << setw(7) << log_F << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  log_φ(F) = k + log_φ(r)\n";
    cout << "  k ∈ ℤ, log_φ(r) ∈ [0, 1)\n";
    cout << "  Ito ay natural na mixed space\n\n";

    return 0;
}
