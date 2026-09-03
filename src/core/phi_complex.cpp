// ============================================
// φ-COMPLEX SPACE
// Normal at log space bilang isang complex number
// z = F + i×log_φ(F)
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <complex>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-COMPLEX SPACE ===\n\n";
    cout << "  z = F + i×log_φ(F)\n\n";
    
    // Addition: z₁ + z₂ = (F₁+F₂) + i×(log₁+log₂)
    // Ito ay nagbibigay ng:
    // - Normal: F₁ + F₂ (addition) ✅
    // - Log: log₁ + log₂ = log_φ(F₁×F₂) (multiplication) ✅
    //
    // Ang problema: ang log ng sum ay hindi log₁ + log₂
    // Kailangan natin ng correction
    
    cout << "  ADDITION SA COMPLEX SPACE\n\n";
    cout << "  F₁ | F₂ | F₁+F₂ | log₁+log₂ | log(F₁+F₂) | Diff\n";
    cout << "  ----|----|-------|-----------|------------|------\n";
    
    for (double F1 : {2.0, 3.0, 5.0}) {
        for (double F2 : {2.0, 3.0, 5.0}) {
            double sum = F1 + F2;
            double log_sum_appx = log(F1)/LN_PHI + log(F2)/LN_PHI;
            double log_sum_exact = log(sum)/LN_PHI;
            double diff = log_sum_appx - log_sum_exact;
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(5) << sum << " | "
                 << setw(10) << fixed << setprecision(3) << log_sum_appx << " | "
                 << setw(10) << log_sum_exact << " | "
                 << setw(7) << diff << "\n";
        }
    }
    
    cout << "\n  MULTIPLICATION SA COMPLEX SPACE\n\n";
    cout << "  F₁ | F₂ | F₁×F₂ | log₁+log₂ | log(F₁×F₂) | Diff\n";
    cout << "  ----|----|-------|-----------|------------|------\n";
    
    for (double F1 : {2.0, 3.0, 5.0}) {
        for (double F2 : {2.0, 3.0, 5.0}) {
            double prod = F1 * F2;
            double log_prod_appx = log(F1)/LN_PHI + log(F2)/LN_PHI;
            double log_prod_exact = log(prod)/LN_PHI;
            double diff = log_prod_appx - log_prod_exact;
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(5) << prod << " | "
                 << setw(10) << fixed << setprecision(3) << log_prod_appx << " | "
                 << setw(10) << log_prod_exact << " | "
                 << setw(7) << diff << "\n";
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang multiplication ay eksakto sa complex space\n";
    cout << "  Ang addition ay may correction na kailangan\n\n";

    return 0;
}
