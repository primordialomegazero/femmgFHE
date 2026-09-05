// ============================================
// φ-CLOSE VALUES
// Ang φ-behavior para sa malapit na values
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-CLOSE VALUES ===\n\n";
    
    // Para sa malapit na values, ang correction ay:
    // correction(d) = log_φ(1 + φ^(-d))
    //
    // Ito ay may φ-structure na maaaring i-decompose
    
    cout << "  d | correction(d) | φ^(-d) | 1+φ^(-d) | log_φ(1+φ^(-d))\n";
    cout << "  --|---------------|---------|----------|-----------------\n";
    
    for (int d = 0; d <= 10; d++) {
        double phi_neg = pow(PHI, -d);
        double one_plus = 1.0 + phi_neg;
        double corr = log(one_plus) / LN_PHI;
        
        cout << "  " << setw(2) << d << " | "
             << setw(10) << fixed << setprecision(4) << corr << " | "
             << setw(8) << phi_neg << " | "
             << setw(8) << one_plus << " | "
             << setw(8) << corr << "\n";
    }
    
    cout << "\n=== ANG PATTERN ===\n\n";
    cout << "  Ang 1+φ^(-d) ay may φ-decomposition:\n";
    cout << "  d=0: 1+1 = 2 = φ + φ⁻¹\n";
    cout << "  d=1: 1+φ⁻¹ = φ\n";
    cout << "  d=2: 1+φ⁻² = φ - φ⁻³\n\n";
    
    cout << "  d | 1+φ^(-d) | φ-decomposition\n";
    cout << "  --|-----------|----------------\n";
    
    for (int d = 0; d <= 5; d++) {
        double val = 1.0 + pow(PHI, -d);
        
        // Hanapin ang φ-decomposition
        double q = floor(val / PHI);
        double r = val - q * PHI;
        
        cout << "  " << d << " | "
             << setw(8) << fixed << setprecision(4) << val << " | "
             << setw(4) << q << "×φ + " << setw(6) << r << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang 1+φ^(-d) ay may natural na φ-decomposition\n";
    cout << "  na maaaring magbigay ng mas malalim na pag-intindi\n";
    cout << "  ng addition para sa malapit na values\n\n";

    return 0;
}
