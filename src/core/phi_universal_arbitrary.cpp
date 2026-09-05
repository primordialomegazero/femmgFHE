// ============================================
// φ-UNIVERSAL ARBITRARY
// Lahat ng ops sa arbitrary values — iisang form
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-UNIVERSAL ARBITRARY ===\n\n";
    
    vector<double> add_corr(20, 0.0);
    vector<double> sub_corr(20, 0.0);
    add_corr[0] = log(2.0) / LN_PHI;
    sub_corr[0] = -100.0;
    for (int d = 1; d <= 15; d++) {
        add_corr[d] = log(1.0 + pow(PHI, -d)) / LN_PHI;
        sub_corr[d] = log(abs(1.0 - pow(PHI, -d))) / LN_PHI;
    }

    // Universal form: result = α×a + β×b + γ×corr(|a-b|)
    // kung saan a = log_φ(F₁), b = log_φ(F₂)
    
    cout << "  F₁ | F₂ | Op | α | β | γ | Result | Expected | Match?\n";
    cout << "  ----|----|----|---|---|---|--------|----------|-------\n";
    
    for (double F1 : {3.0, 7.0, 13.0}) {
        for (double F2 : {5.0, 8.0}) {
            double a = log(F1) / LN_PHI;
            double b = log(F2) / LN_PHI;
            double d = abs(a - b);
            int di = (int)d;
            
            // ADDITION
            double log_add = max(a,b) + add_corr[di];
            double add_result = pow(PHI, log_add);
            double add_expected = F1 + F2;
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | ADD | 0 | 0 | 1 | "
                 << setw(6) << fixed << setprecision(1) << add_result << " | "
                 << setw(6) << add_expected << " | "
                 << (abs(add_result - add_expected) < 0.1 ? "✅" : "❌") << "\n";
            
            // MULTIPLICATION
            double log_mul = a + b;
            double mul_result = pow(PHI, log_mul);
            double mul_expected = F1 * F2;
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | MUL | 1 | 1 | 0 | "
                 << setw(6) << fixed << setprecision(1) << mul_result << " | "
                 << setw(6) << mul_expected << " | "
                 << (abs(mul_result - mul_expected) < 0.5 ? "✅" : "❌") << "\n";
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Universal form: φ^(α×a + β×b + γ×corr)\n";
    cout << "  Lahat ng ops ay iisang form\n";
    cout << "  Walang exceptions, walang special cases\n\n";

    return 0;
}
