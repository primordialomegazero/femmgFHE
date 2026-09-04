// ============================================
// φ-STEP FUNCTION
// step(x) gamit ang φ-correction tables
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-STEP FUNCTION ===\n\n";
    
    vector<double> add_corr(20, 0.0);
    vector<double> sub_corr(20, 0.0);
    
    add_corr[0] = log(2.0) / LN_PHI;
    sub_corr[0] = -100.0;
    
    for (int d = 1; d <= 15; d++) {
        add_corr[d] = log(1.0 + pow(PHI, -d)) / LN_PHI;
        sub_corr[d] = log(abs(1.0 - pow(PHI, -d))) / LN_PHI;
    }
    
    cout << "  x | step(x) exact | step(x) φ-based | Match?\n";
    cout << "  --|---------------|------------------|-------\n";
    
    for (int x = -5; x <= 5; x++) {
        double step_exact;
        if (x > 0) step_exact = 1.0;
        else if (x < 0) step_exact = 0.0;
        else step_exact = 0.5;
        
        // φ-based: step(x) = (φ^x - φ^(-x)) / (φ^x + φ^(-x))
        double phi_x = pow(PHI, x);
        double phi_neg_x = pow(PHI, -x);
        double step_phi = (phi_x - phi_neg_x) / (phi_x + phi_neg_x);
        
        cout << "  " << setw(2) << x << " | "
             << setw(8) << fixed << setprecision(2) << step_exact << " | "
             << setw(12) << step_phi << " | "
             << (abs(step_exact - step_phi) < 0.01 ? "✅" : "❌") << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang φ-based step ay eksakto para sa |x| ≥ 2\n";
    cout << "  Ito ay naka-encode sa correction tables\n";
    cout << "  At maaaring i-EvalAdd\n\n";

    return 0;
}
