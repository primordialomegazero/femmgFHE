// ============================================
// φ-ENDOMORPHISM
// Ang φ bilang linear transformation
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    
    cout << "=== φ-ENDOMORPHISM ===\n\n";
    
    // Ang φ ay may property:
    // φ(x) = (x + √(x² + 4)) / 2
    // Ito ay ang fixed point ng transformation:
    // x → (x + √(x² + 4)) / 2
    
    cout << "  φ(x) = (x + √(x² + 4)) / 2\n\n";
    
    cout << "  x | φ(x) | φ(φ(x)) | φ(φ(φ(x)))\n";
    cout << "  --|------|---------|----------\n";
    
    for (double x : {0.0, 0.5, 1.0, 1.5, 2.0}) {
        double fx = (x + sqrt(x*x + 4.0)) / 2.0;
        double ffx = (fx + sqrt(fx*fx + 4.0)) / 2.0;
        double fffx = (ffx + sqrt(ffx*ffx + 4.0)) / 2.0;
        
        cout << "  " << setw(4) << x << " | "
             << setw(7) << fixed << setprecision(3) << fx << " | "
             << setw(8) << ffx << " | "
             << setw(8) << fffx << "\n";
    }
    
    cout << "\n  Ang fixed point ay φ = 1.618...\n\n";
    
    // ============================================
    // ANG ENDOMORPHISM BILANG OPERATOR
    // ============================================
    cout << "=== ENDOMORPHISM BILANG OPERATOR ===\n\n";
    cout << "  Kung ang operasyon ay naka-encode bilang:\n";
    cout << "  F₁ ⊙ F₂ = φ(φ⁻¹(F₁) + φ⁻¹(F₂))\n\n";
    
    cout << "  F₁ | F₂ | φ⁻¹(F₁) | φ⁻¹(F₂) | Sum | φ(Sum)\n";
    cout << "  ----|----|----------|----------|-----|-------\n";
    
    for (double F1 : {3.0, 5.0, 8.0}) {
        for (double F2 : {4.0, 7.0}) {
            double inv1 = 2.0 * F1 - sqrt(4.0 * F1 * F1 + 4.0);
            inv1 = inv1 / 2.0;
            double inv2 = 2.0 * F2 - sqrt(4.0 * F2 * F2 + 4.0);
            inv2 = inv2 / 2.0;
            double sum = inv1 + inv2;
            double result = (sum + sqrt(sum * sum + 4.0)) / 2.0;
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(7) << fixed << setprecision(2) << inv1 << " | "
                 << setw(7) << inv2 << " | "
                 << setw(5) << sum << " | "
                 << setw(7) << result << "\n";
        }
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang endomorphism ay may natural na inverse\n";
    cout << "  na maaaring magbigay ng universal form\n\n";

    return 0;
}
