// ============================================
// φ-MAX ADDITION
// F₁ ⊕ F₂ = max(F₁, F₂)
// n-space: max(n₁, n₂) — walang correction
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-MAX ADDITION ===\n\n";
    cout << "  F₁ ⊕ F₂ = max(F₁, F₂)\n";
    cout << "  n₁ ⊕ n₂ = max(n₁, n₂)\n\n";
    
    cout << "  F₁ | F₂ | max(F₁,F₂) | n₁ | n₂ | max(n₁,n₂) | Match?\n";
    cout << "  ----|----|-------------|-----|-----|-------------|-------\n";
    
    for (double F1 : {3.0, 7.0, 13.0, 21.0}) {
        for (double F2 : {5.0, 8.0, 11.0}) {
            double max_F = max(F1, F2);
            double n1 = log(F1) / LN_PHI;
            double n2 = log(F2) / LN_PHI;
            double max_n = max(n1, n2);
            double recon = pow(PHI, max_n);
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(7) << max_F << " | "
                 << setw(6) << fixed << setprecision(3) << n1 << " | "
                 << setw(5) << n2 << " | "
                 << setw(6) << max_n << " | "
                 << (abs(recon - max_F) < 0.01 ? "✅" : "❌") << "\n";
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang max addition ay natural sa n-space\n";
    cout << "  Walang correction — ang max ay implicit\n";
    cout << "  sa φ-structure ng mga values\n\n";

    return 0;
}
