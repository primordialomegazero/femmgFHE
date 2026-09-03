// ============================================
// φ-CARRY BASIS
// Hanapin ang φ-basis na may natural na carry
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-CARRY BASIS ===\n\n";

    // ============================================
    // 1. CARRY BILANG φ-POWER
    // ============================================
    cout << "--- 1. CARRY BILANG φ-POWER ---\n\n";
    cout << "  Ang carry ay nangyayari kapag sum ≥ φ\n";
    cout << "  Sa φ-power terms: φ¹ = 1.618, φ⁰ = 1\n\n";
    
    cout << "  sum | sum/φ | floor(sum/φ) | carry\n";
    cout << "  -----|-------|--------------|-------\n";
    
    for (double sum : {0.5, 0.8, 1.0, 1.2, 1.5, 1.6, 1.7, 2.0, 2.5, 3.0}) {
        double ratio = sum / PHI;
        double floor_ratio = floor(ratio);
        bool carry = floor_ratio >= 1;
        
        cout << "  " << setw(4) << fixed << setprecision(1) << sum << " | "
             << setw(5) << ratio << " | "
             << setw(6) << floor_ratio << " | "
             << (carry ? "1" : "0") << "\n";
    }

    // ============================================
    // 2. ANG φ-BASIS NG CARRY
    // ============================================
    cout << "\n--- 2. ANG φ-BASIS NG CARRY ---\n\n";
    cout << "  Kung ang carry ay naka-encode sa φ-basis:\n";
    cout << "  carry = 1 kapag sum ≥ φ\n";
    cout << "  carry = 0 kapag sum < φ\n\n";
    
    cout << "  Ito ay katumbas ng:\n";
    cout << "  carry = floor(sum × φ⁻¹)\n\n";
    
    cout << "  sum | sum × φ⁻¹ | floor | carry\n";
    cout << "  -----|-----------|-------|-------\n";
    
    for (double sum : {0.5, 0.8, 1.0, 1.2, 1.5, 1.6, 1.7, 2.0, 2.5, 3.0}) {
        double mul = sum * (PHI - 1.0);  // φ⁻¹ = φ - 1
        double fl = floor(mul);
        bool carry = fl >= 1;
        
        cout << "  " << setw(4) << fixed << setprecision(1) << sum << " | "
             << setw(7) << mul << " | "
             << setw(5) << fl << " | "
             << (carry ? "1" : "0") << "\n";
    }

    // ============================================
    // 3. φ-BASIS NA WALANG COMPARISON
    // ============================================
    cout << "\n--- 3. φ-BASIS NA WALANG COMPARISON ---\n\n";
    cout << "  Ang floor(x) ay maaaring i-express bilang:\n";
    cout << "  floor(x) = x - fmod(x, 1)\n\n";
    cout << "  At ang fmod(x, 1) ay periodic sa φ-basis\n\n";
    
    cout << "  sum | fmod(sum/φ, 1) | 1 - fmod | approx carry\n";
    cout << "  -----|----------------|-----------|-------------\n";
    
    for (double sum : {0.5, 0.8, 1.0, 1.2, 1.5, 1.6, 1.7, 2.0, 2.5, 3.0}) {
        double fm = fmod(sum / PHI, 1.0);
        double approx_carry = 1.0 - fm;
        
        cout << "  " << setw(4) << fixed << setprecision(1) << sum << " | "
             << setw(8) << fm << " | "
             << setw(8) << approx_carry << " | "
             << setw(8) << (sum >= PHI ? 1.0 : 0.0) << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Hanapin kung ang carry ay may φ-basis na\n";
    cout << "  pure EvalAdd/EvalSub na walang comparison\n\n";

    return 0;
}
