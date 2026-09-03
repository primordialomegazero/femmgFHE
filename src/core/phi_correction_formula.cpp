// ============================================
// φ-CORRECTION FORMULA
// Hanapin ang simpleng formula para sa
// addition correction sa complex space
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-CORRECTION FORMULA ===\n\n";
    cout << "  Correction = log_φ(1/F₁ + 1/F₂)\n\n";
    
    cout << "  F₁ | F₂ | 1/F₁+1/F₂ | log_φ(1/F₁+1/F₂) | Pattern?\n";
    cout << "  ----|----|-----------|-------------------|--------\n";
    
    for (double F1 : {2.0, 3.0, 5.0, 8.0}) {
        for (double F2 : {2.0, 3.0, 5.0, 8.0}) {
            double reciprocal_sum = 1.0/F1 + 1.0/F2;
            double corr = log(reciprocal_sum) / LN_PHI;
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(8) << fixed << setprecision(4) << reciprocal_sum << " | "
                 << setw(12) << corr << " | "
                 << setw(6) << (abs(corr - round(corr)) < 0.1 ? "✅" : "  ") << "\n";
        }
    }
    
    cout << "\n=== FIBONACCI RECIPROCALS ===\n\n";
    cout << "  1/F_n + 1/F_{n+1} = ?\n\n";
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    cout << "  n | F_n | F_{n+1} | 1/F_n + 1/F_{n+1} | ≈ ?\n";
    cout << "  --|-----|---------|-------------------|-----\n";
    
    for (int n = 2; n <= 10; n++) {
        double F_n = (double)fib[n];
        double F_np1 = (double)fib[n+1];
        double recip_sum = 1.0/F_n + 1.0/F_np1;
        
        cout << "  " << setw(2) << n << " | "
             << setw(4) << fib[n] << " | "
             << setw(7) << fib[n+1] << " | "
             << setw(12) << fixed << setprecision(6) << recip_sum << " | "
             << setw(6) << (abs(recip_sum - (1.0/(PHI-1.0))) < 0.01 ? "✅" : "  ")
             << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Kung ang reciprocal sum ay may simpleng pattern,\n";
    cout << "  ang correction ay additive constant\n\n";

    return 0;
}
