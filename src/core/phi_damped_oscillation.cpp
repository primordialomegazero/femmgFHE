// ============================================
// φ-DAMPED OSCILLATION
// Ang even/odd convergence ng φ-powers
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double PHI_INV = 1.0 / PHI;

    cout << "=== φ-DAMPED OSCILLATION ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Ang convergence ng frac(φ^n)
    // Even n → 1 (mula sa itaas)
    // Odd n → 0 (mula sa ibaba)
    // ============================================
    cout << "--- 1. Convergence ng frac(φ^n) ---\n\n";
    cout << "  n | frac(φ^n) | Distance sa limit\n";
    cout << "  --|-----------|------------------\n";
    
    for (int n = 0; n <= 40; n++) {
        double phi_n = pow(PHI, n);
        double frac = phi_n - floor(phi_n);
        
        double limit = (n % 2 == 0) ? 1.0 : 0.0;
        double dist = abs(frac - limit);
        
        if (n <= 10 || n % 5 == 0) {
            cout << "  " << setw(2) << n << " | "
                 << setw(17) << frac << " | "
                 << setw(17) << dist << "\n";
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang convergence rate — φ-powered?
    // ============================================
    cout << "--- 2. Convergence rate ---\n\n";
    cout << "  n | dist | dist × φ^n | Ratio\n";
    cout << "  --|------|-------------|-------\n";
    
    double prev_dist_even = 0, prev_dist_odd = 0;
    for (int n = 0; n <= 20; n++) {
        double phi_n = pow(PHI, n);
        double frac = phi_n - floor(phi_n);
        double limit = (n % 2 == 0) ? 1.0 : 0.0;
        double dist = abs(frac - limit);
        
        double scaled = dist * pow(PHI, n);
        
        string ratio_str = "-";
        if (n >= 2) {
            double prev = (n >= 4) ? prev_dist_even : prev_dist_odd;
            if (prev > 0) {
                ratio_str = to_string(dist / prev);
            }
        }
        
        if (n % 2 == 0) prev_dist_even = dist;
        else prev_dist_odd = dist;
        
        cout << "  " << setw(2) << n << " | "
             << setw(14) << dist << " | "
             << setw(15) << scaled << " | "
             << setw(12) << ratio_str << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang oscillation amplitude
    // ============================================
    cout << "--- 3. Oscillation amplitude ---\n\n";
    cout << "  Ang distance sa limit ay may φ⁻² decay:\n\n";
    
    for (int n = 0; n <= 20; n++) {
        double phi_n = pow(PHI, n);
        double frac = phi_n - floor(phi_n);
        double limit = (n % 2 == 0) ? 1.0 : 0.0;
        double dist = abs(frac - limit);
        
        double phi_decay = pow(PHI, -2.0 * n);
        
        cout << "  " << setw(2) << n << " | "
             << setw(14) << dist << " | "
             << setw(14) << phi_decay << " | "
             << setw(10) << (dist / phi_decay) << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang two-state system
    // ============================================
    cout << "--- 4. Two-state system ---\n\n";
    cout << "  Even n → 1 (state A)\n";
    cout << "  Odd n → 0 (state B)\n";
    cout << "  Ang φ^n ay may natural na binary states\n\n";
    
    cout << "  State A (even): frac → 1\n";
    cout << "  State B (odd): frac → 0\n\n";
    
    cout << "  Transition: A → B → A → B → ...\n";
    cout << "  Ito ay natural na binary counter\n";
    cout << "  Ang periodicity ay 2 (even/odd)\n\n";

    // ============================================
    // 5. Ang anchoring property
    // ============================================
    cout << "--- 5. Anchoring property ---\n\n";
    cout << "  φ^n + φ^(n+1) = φ^(n+2)\n";
    cout << "  Ito ay exact sa lahat ng n\n\n";
    
    cout << "  Check:\n";
    for (int n = 0; n <= 10; n++) {
        double phi_n = pow(PHI, n);
        double phi_n1 = pow(PHI, n+1);
        double phi_n2 = pow(PHI, n+2);
        
        cout << "  n=" << setw(2) << n << ": "
             << "φ^" << n << " + φ^" << n+1 << " = "
             << phi_n + phi_n1 << " | φ^" << n+2 << " = "
             << phi_n2 << " | "
             << (abs(phi_n + phi_n1 - phi_n2) < 1e-10 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 6. Ang fractional part bilang phase
    // ============================================
    cout << "--- 6. Fractional part bilang phase ---\n\n";
    cout << "  frac(φ^n) ay may alternating phase:\n";
    cout << "  Phase 0 (even): papunta sa 1\n";
    cout << "  Phase 1 (odd): papunta sa 0\n\n";
    
    cout << "  n | phase | frac | sin(π×n/2)\n";
    cout << "  --|-------|------|------------\n";
    
    for (int n = 0; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double frac = phi_n - floor(phi_n);
        int phase = n % 2;
        double sin_val = sin(M_PI * n / 2.0);
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << phase << " | "
             << setw(14) << frac << " | "
             << setw(14) << sin_val << "\n";
    }
    cout << "\n";

    return 0;
}
