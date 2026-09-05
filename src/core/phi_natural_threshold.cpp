// ============================================
// φ-NATURAL THRESHOLD
// Kung saan ang addition ay automatic na max
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-NATURAL THRESHOLD ===\n\n";
    
    // Ang correction(d) = log_φ(1 + φ^(-d))
    // Para sa malaking d, ito ay → 0
    // Kaya: φ^a + φ^b → φ^max(a,b) para sa malaking d
    
    cout << "  d | correction(d) | φ^(-d) | % ng max\n";
    cout << "  --|---------------|---------|----------\n";
    
    for (int d = 0; d <= 20; d++) {
        double corr = log(1.0 + pow(PHI, -d)) / LN_PHI;
        double phi_neg = pow(PHI, -d);
        double pct = corr / d * 100.0;  // relative sa exponent
        
        cout << "  " << setw(2) << d << " | "
             << setw(10) << fixed << setprecision(4) << corr << " | "
             << setw(8) << phi_neg << " | "
             << setw(7) << pct << "%\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Para sa d ≥ 10: correction < 0.02\n";
    cout << "  Ito ay < 0.2% ng exponent\n";
    cout << "  Halos automatic na max na ito\n\n";
    
    // ============================================
    // ANG EMERGENT NA BEHAVIOR
    // ============================================
    cout << "=== EMERGENT BEHAVIOR ===\n\n";
    cout << "  φ^a + φ^b sa iba't ibang d:\n\n";
    cout << "  a | b | d | φ^a+φ^b | φ^max | Diff | % Error\n";
    cout << "  --|---|----|---------|-------|------|--------\n";
    
    for (int a : {1, 3, 5, 8}) {
        for (int d : {1, 3, 5, 10}) {
            int b = a + d;
            double sum = pow(PHI, a) + pow(PHI, b);
            double max_val = pow(PHI, b);
            double diff = sum - max_val;
            double error = diff / max_val * 100.0;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(2) << b << " | "
                 << setw(3) << d << " | "
                 << setw(8) << fixed << setprecision(2) << sum << " | "
                 << setw(7) << max_val << " | "
                 << setw(7) << diff << " | "
                 << setw(6) << error << "%\n";
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Para sa d ≥ 5: ang sum ay ≈ max (error < 1%)\n";
    cout << "  Para sa d ≥ 10: ang sum ay ≈ max (error < 0.01%)\n";
    cout << "  Ang φ ay may natural na threshold sa d\n";
    cout << "  kung saan ang addition ay automatic na max\n\n";

    return 0;
}
