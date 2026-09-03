// ============================================
// φ-MOD1 LOGSPACE
// Hanapin ang koneksyon ng mod 1 space sa log space
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-MOD1 LOGSPACE ===\n\n";

    // ============================================
    // 1. fmod(φ^n, 1) AT log_φ
    // ============================================
    cout << "--- 1. fmod(φ^n, 1) AT log_φ ---\n\n";
    cout << "  n | fmod(φ^n,1) | log_φ(fmod) | n | Diff\n";
    cout << "  --|-------------|-------------|---|------\n";
    
    for (int n = 1; n <= 15; n++) {
        double fm = fmod(pow(PHI, n), 1.0);
        double log_fm = (fm > 0) ? log(fm) / LN_PHI : -100;
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << fixed << setprecision(4) << fm << " | "
             << setw(10) << log_fm << " | "
             << setw(3) << n << " | "
             << setw(6) << (log_fm - n) << "\n";
    }

    // ============================================
    // 2. ANG TWO-STATE PATTERN
    // ============================================
    cout << "\n--- 2. TWO-STATE PATTERN ---\n\n";
    cout << "  fmod(φ^n, 1) = φ⁻ⁿ (odd n)\n";
    cout << "  fmod(φ^n, 1) = 1 - φ⁻ⁿ (even n)\n\n";
    
    cout << "  n | fmod | φ⁻ⁿ | 1-φ⁻ⁿ | Match?\n";
    cout << "  --|------|-----|-------|-------\n";
    
    for (int n = 1; n <= 10; n++) {
        double fm = fmod(pow(PHI, n), 1.0);
        double phi_neg_n = pow(PHI, -n);
        double one_minus = 1.0 - phi_neg_n;
        
        double match_val = (n % 2 == 1) ? phi_neg_n : one_minus;
        
        cout << "  " << setw(2) << n << " | "
             << setw(7) << fixed << setprecision(4) << fm << " | "
             << setw(7) << phi_neg_n << " | "
             << setw(7) << one_minus << " | "
             << (abs(fm - match_val) < 0.01 ? "✅" : "❌") << "\n";
    }

    // ============================================
    // 3. ANG CARRY SA MOD 1 SPACE
    // ============================================
    cout << "\n--- 3. CARRY SA MOD 1 SPACE ---\n\n";
    cout << "  Kapag fmod(F₁,1) + fmod(F₂,1) ≥ 1, may carry\n";
    cout << "  carry = floor(fmod(F₁,1) + fmod(F₂,1))\n\n";
    
    cout << "  fmod₁ | fmod₂ | Sum | floor | carry\n";
    cout << "  -------|-------|-----|-------|-------\n";
    
    for (double fm1 : {0.2, 0.5, 0.8, 1.2}) {
        for (double fm2 : {0.3, 0.5, 0.7}) {
            double sum = fmod(fm1, 1.0) + fmod(fm2, 1.0);
            double fl = floor(sum);
            bool carry = fl >= 1;
            
            cout << "  " << setw(5) << fixed << setprecision(1) << fmod(fm1, 1.0) << " | "
                 << setw(5) << fmod(fm2, 1.0) << " | "
                 << setw(5) << sum << " | "
                 << setw(5) << fl << " | "
                 << (carry ? "1" : "0") << "\n";
        }
    }

    // ============================================
    // 4. ANG BRIDGE SA LOG SPACE
    // ============================================
    cout << "\n--- 4. BRIDGE SA LOG SPACE ---\n\n";
    cout << "  Ang fmod(φ^n, 1) ay may log_φ na ≈ -n (odd)\n";
    cout << "  o ≈ 0 (even, kasi 1-φ⁻ⁿ ≈ 1, log_φ(1)=0)\n\n";
    
    cout << "  Ito ay nagbibigay ng natural na index:\n";
    cout << "  - Odd n: log_φ(fmod) ≈ -n\n";
    cout << "  - Even n: log_φ(fmod) ≈ 0\n\n";
    
    cout << "=== KEY ===\n";
    cout << "  Ang mod 1 space ay may two-state na φ-pattern\n";
    cout << "  Pero ang log space bridge ay hindi pa malinaw\n\n";

    return 0;
}
