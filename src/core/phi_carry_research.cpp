// ============================================
// φ-CARRY RESEARCH
// I-verify ang d = |a-b| at correction(d)
// sa plaintext muna bago i-FHE
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-CARRY RESEARCH ===\n\n";

    // ============================================
    // 1. d = |a-b| mula sa index
    // ============================================
    cout << "--- 1. d = |a-b| MULA SA INDEX ---\n\n";
    cout << "  a | b | d = |a-b| | EvalSub | Match?\n";
    cout << "  --|---|---------|---------|-------\n";
    
    for (int a : {1, 3, 5, 8, 13}) {
        for (int b : {a+1, a+2, a+3}) {
            int d = abs(a - b);
            int eval_sub = abs(a - b);  // EvalSub sa Slot 3
            
            cout << "  " << setw(2) << a << " | "
                 << setw(2) << b << " | "
                 << setw(3) << d << " | "
                 << setw(4) << eval_sub << " | "
                 << (d == eval_sub ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // 2. correction(d) ≈ φ^(-d)
    // ============================================
    cout << "\n--- 2. correction(d) ≈ φ^(-d) ---\n\n";
    cout << "  d | correction(d) | φ^(-d) | Diff | Error%\n";
    cout << "  --|---------------|---------|------|-------\n";
    
    for (int d = 0; d <= 10; d++) {
        double corr = log(1.0 + pow(PHI, -d)) / LN_PHI;
        double approx = pow(PHI, -d);
        double diff = corr - approx;
        double error = abs(diff) / corr * 100.0;
        
        cout << "  " << setw(2) << d << " | "
             << setw(10) << fixed << setprecision(4) << corr << " | "
             << setw(8) << approx << " | "
             << setw(7) << diff << " | "
             << setw(6) << error << "%\n";
    }

    // ============================================
    // 3. CARRY NA MAY CORRECTION
    // ============================================
    cout << "\n--- 3. CARRY NA MAY CORRECTION ---\n\n";
    cout << "  Kapag r_old + r_addend ≥ φ, may carry\n";
    cout << "  Ang carry ay: q_new = q_old + 1\n\n";
    
    cout << "  r_old | r_add | Sum | ≥ φ? | Carry?\n";
    cout << "  -------|-------|-----|------|-------\n";
    
    for (double r_old : {0.1, 0.3, 0.5, 0.8, 1.0, 1.2}) {
        for (double r_add : {0.1, 0.3, 0.5, 0.8}) {
            double sum = r_old + r_add;
            bool has_carry = sum >= PHI;
            
            cout << "  " << setw(5) << fixed << setprecision(1) << r_old << " | "
                 << setw(5) << r_add << " | "
                 << setw(5) << sum << " | "
                 << (has_carry ? "YES" : " no") << " | "
                 << (has_carry ? "+1" : " 0") << "\n";
        }
    }

    cout << "\n=== KEY ===\n";
    cout << "  Kung ang correction ≈ φ^(-d) ay may maliit na error,\n";
    cout << "  maaari nating gamitin ito para sa carry\n\n";

    return 0;
}
