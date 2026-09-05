// ============================================
// φ-HARD STEP
// Hanapin ang hard ±1 step na walang EvalMult
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-HARD STEP ===\n\n";
    
    cout << "  δ_log | φ^δ - φ^-δ | φ^δ + φ^-δ | Step\n";
    cout << "  -------|------------|------------|------\n";
    
    for (double dl : {-3.0, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 3.0}) {
        double phi_p = pow(PHI, dl);
        double phi_n = pow(PHI, -dl);
        double diff = phi_p - phi_n;
        double sum = phi_p + phi_n;
        double step = diff / sum;
        
        cout << "  " << setw(7) << fixed << setprecision(1) << dl << " | "
             << setw(10) << diff << " | "
             << setw(10) << sum << " | "
             << setw(7) << step << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang (φ^δ - φ^-δ)/(φ^δ + φ^-δ) ay\n";
    cout << "  mas malapit sa hard step para sa malaking |δ|\n";
    cout << "  Ngunit kailangan pa ng EvalMult para sa ratio\n\n";
    
    // ============================================
    // ALTERNATIVE: GAMIT ANG SLOT 1 DIRECTLY
    // ============================================
    cout << "=== ALTERNATIVE: SLOT 1 DIRECT ===\n\n";
    cout << "  Ang Slot 1 = F ay may natural na threshold\n";
    cout << "  F > φ^5 → branch A\n";
    cout << "  F ≤ φ^5 → branch B\n\n";
    
    cout << "  Ito ay automatic — walang computation\n";
    cout << "  Ang branch ay naka-encode sa φ-structure\n\n";

    return 0;
}
