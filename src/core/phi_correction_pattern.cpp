// ============================================
// φ-CORRECTION PATTERN
// Hanapin ang eksaktong pattern ng correction
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-CORRECTION PATTERN ===\n\n";
    
    // Ang correction(d) = log_φ(1 + φ^(-d))
    // Hanapin ang eksaktong relasyon sa φ-powers
    
    cout << "  d | corr(d) | φ^(1-d) | corr/φ^(1-d) | Pattern\n";
    cout << "  --|---------|---------|--------------|--------\n";
    
    for (int d = 1; d <= 15; d++) {
        double corr = log(1.0 + pow(PHI, -d)) / LN_PHI;
        double phi_power = pow(PHI, 1 - d);
        double ratio = corr / phi_power;
        
        cout << "  " << setw(2) << d << " | "
             << setw(8) << fixed << setprecision(4) << corr << " | "
             << setw(8) << phi_power << " | "
             << setw(8) << ratio << " | "
             << setw(6) << (abs(ratio - 1.0) < 0.1 ? "≈1" : "?") << "\n";
    }
    
    cout << "\n=== ALTERNATIVE PATTERN ===\n\n";
    cout << "  corr(d) × φ^d = ?\n\n";
    
    cout << "  d | corr(d) | corr(d)×φ^d | Pattern\n";
    cout << "  --|---------|--------------|--------\n";
    
    for (int d = 0; d <= 15; d++) {
        double corr = log(1.0 + pow(PHI, -d)) / LN_PHI;
        double scaled = corr * pow(PHI, d);
        
        cout << "  " << setw(2) << d << " | "
             << setw(8) << fixed << setprecision(4) << corr << " | "
             << setw(8) << scaled << " | "
             << setw(8) << (abs(scaled - 1.0) < 0.1 ? "≈1" : "?") << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Hanapin kung aling scaling ang nagbibigay\n";
    cout << "  ng constant na pattern\n\n";

    return 0;
}
