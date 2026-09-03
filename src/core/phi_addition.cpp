// ============================================
// φ-ADDITION
// φ-addition: F₁ ⊕ F₂ = φ × max(F₁, F₂)
// Additive sa log space
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-ADDITION ===\n\n";
    cout << "  F₁ ⊕ F₂ = φ × max(F₁, F₂)\n\n";
    
    cout << "  F₁ | F₂ | φ×max | log_φ(F₁⊕F₂) | max(log₁,log₂)+1 | Diff\n";
    cout << "  ----|----|-------|--------------|-------------------|------\n";
    
    for (double F1 : {2.0, 3.0, 5.0, 8.0, 13.0}) {
        for (double F2 : {3.0, 5.0, 8.0}) {
            double phi_max = PHI * max(F1, F2);
            double log_phi_max = log(phi_max) / LN_PHI;
            double max_log_plus_1 = max(log(F1)/LN_PHI, log(F2)/LN_PHI) + 1.0;
            double diff = log_phi_max - max_log_plus_1;
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(5) << fixed << setprecision(1) << phi_max << " | "
                 << setw(8) << log_phi_max << " | "
                 << setw(10) << max_log_plus_1 << " | "
                 << setw(7) << diff << "\n";
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Kung ang diff ay 0,\n";
    cout << "  ang φ-addition ay additive sa log space\n";
    cout << "  at multiplicative sa normal space\n\n";

    return 0;
}
