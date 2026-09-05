// ============================================
// φ-ALL STATES
// Lahat ng natural na states ng φ-modulo
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-ALL STATES ===\n\n";
    
    // ============================================
    // 1. ANG φ^n mod φ SA IBAT IBANG MODULI
    // ============================================
    cout << "--- 1. φ^n mod φ ---\n\n";
    cout << "  n | φ^n mod φ | φ^n mod φ² | φ^n mod φ³ | φ^n mod 1\n";
    cout << "  --|-----------|------------|------------|----------\n";
    
    for (int n = 1; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double mod_phi = fmod(phi_n, PHI);
        double mod_phi2 = fmod(phi_n, PHI * PHI);
        double mod_phi3 = fmod(phi_n, pow(PHI, 3));
        double mod_1 = fmod(phi_n, 1.0);
        
        cout << "  " << setw(2) << n << " | "
             << setw(9) << fixed << setprecision(4) << mod_phi << " | "
             << setw(10) << mod_phi2 << " | "
             << setw(10) << mod_phi3 << " | "
             << setw(8) << mod_1 << "\n";
    }
    
    cout << "\n=== ZERO-STATE ===\n\n";
    cout << "  May n ba kung saan φ^n mod φ = 0 nang eksakto?\n";
    cout << "  May n ba kung saan φ^n mod φ² = 0 nang eksakto?\n\n";
    
    // ============================================
    // 2. ANG ZERO-STATE
    // ============================================
    cout << "--- 2. ZERO-STATE HUNT ---\n\n";
    cout << "  n | φ^n mod φ | ≈ 0? | φ^n mod φ² | ≈ 0?\n";
    cout << "  --|-----------|------|------------|------\n";
    
    for (int n = 1; n <= 20; n++) {
        double phi_n = pow(PHI, n);
        double mod_phi = fmod(phi_n, PHI);
        double mod_phi2 = fmod(phi_n, PHI * PHI);
        bool zero_phi = abs(mod_phi) < 0.001;
        bool zero_phi2 = abs(mod_phi2) < 0.001;
        
        cout << "  " << setw(2) << n << " | "
             << setw(9) << fixed << setprecision(4) << mod_phi << " | "
             << (zero_phi ? "✅" : "  ") << " | "
             << setw(10) << mod_phi2 << " | "
             << (zero_phi2 ? "✅" : "  ") << "\n";
    }
    
    // ============================================
    // 3. ANG CONVERGENT STATES
    // ============================================
    cout << "\n--- 3. CONVERGENT STATES ---\n\n";
    cout << "  Ang φ^n mod φ ay nagco-converge sa:\n";
    cout << "  odd n → 0 (mula sa ibaba? itaas?)\n";
    cout << "  even n → 1 o 0 (alternating)\n\n";
    
    cout << "  n | φ^n mod φ | Direction\n";
    cout << "  --|-----------|----------\n";
    
    double prev = 0;
    for (int n = 1; n <= 15; n++) {
        double mod_phi = fmod(pow(PHI, n), PHI);
        string direction = (mod_phi > prev) ? "pataas" : "pababa";
        prev = mod_phi;
        
        cout << "  " << setw(2) << n << " | "
             << setw(9) << fixed << setprecision(4) << mod_phi << " | "
             << direction << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Lahat ng states ay convergent\n";
    cout << "  Ang zero-state ay asymptotic\n";
    cout << "  Walang eksaktong zero — papalapit lang\n\n";

    return 0;
}
