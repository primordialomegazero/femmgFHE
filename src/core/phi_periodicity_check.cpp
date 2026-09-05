// ============================================
// φ-PERIODICITY CHECK
// Hanapin ang natural na periodicity ng φ
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <complex>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-PERIODICITY CHECK ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. φ^n mod φ
    // ============================================
    cout << "--- 1. φ^n mod φ ---\n\n";
    cout << "  n | φ^n | φ^n mod φ | Pattern\n";
    cout << "  --|-----|-----------|--------\n";
    
    for (int n = 0; n <= 20; n++) {
        double phi_n = pow(PHI, n);
        double mod_phi = fmod(phi_n, PHI);
        
        string pattern;
        if (n % 2 == 0) {
            pattern = (abs(mod_phi - 1.0) < 0.01) ? "→ 1" : "→ 0";
        } else {
            pattern = (abs(mod_phi - PHI + 1.0) < 0.01) ? "→ φ⁻¹" : "→ φ";
        }
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << phi_n << " | "
             << setw(12) << mod_phi << " | "
             << pattern << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Fractional part ng φ^n
    // ============================================
    cout << "--- 2. Fractional part ng φ^n ---\n\n";
    cout << "  n | frac(φ^n) | Direction\n";
    cout << "  --|-----------|----------\n";
    
    for (int n = 0; n <= 30; n++) {
        double phi_n = pow(PHI, n);
        double frac = phi_n - floor(phi_n);
        
        string dir = (frac > 0.5) ? "↑ > 0.5" : "↓ < 0.5";
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << frac << " | "
             << dir << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. φ^n sa log space — periodicity?
    // ============================================
    cout << "--- 3. φ^n sa log space ---\n\n";
    cout << "  n | log_φ(φ^n) | n mod 1 | n mod φ | n mod φ²\n";
    cout << "  --|------------|---------|---------|----------\n";
    
    for (int n = 0; n <= 20; n++) {
        double mod1 = fmod((double)n, 1.0);
        double mod_phi = fmod((double)n, PHI);
        double mod_phi2 = fmod((double)n, PHI * PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << (double)n << " | "
             << setw(8) << mod1 << " | "
             << setw(10) << mod_phi << " | "
             << setw(10) << mod_phi2 << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang correction term periodicity
    // ============================================
    cout << "--- 4. Correction term periodicity ---\n\n";
    cout << "  n | corr(n) | corr(n+1) | diff\n";
    cout << "  --|---------|-----------|------\n";
    
    for (int n = 0; n <= 15; n++) {
        double corr = log(1.0 + pow(PHI, -n)) / LN_PHI;
        double corr_next = log(1.0 + pow(PHI, -(n+1))) / LN_PHI;
        double diff = corr_next - corr;
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << corr << " | "
             << setw(10) << corr_next << " | "
             << setw(10) << diff << "\n";
    }
    cout << "\n";

    // ============================================
    // 5. φ bilang period sa exponent
    // ============================================
    cout << "--- 5. φ bilang period sa exponent ---\n\n";
    cout << "  φ^φ = " << pow(PHI, PHI) << "\n";
    cout << "  φ^(n+φ) / φ^n = " << pow(PHI, PHI) << " (constant)\n";
    cout << "  Kung mag-add ng φ sa exponent: multiply ng φ^φ\n\n";

    // ============================================
    // 6. Natural na cycle ng φ-powers
    // ============================================
    cout << "--- 6. Natural na cycle ng φ-powers ---\n\n";
    cout << "  φ^0 = 1\n";
    cout << "  φ^1 = φ\n";
    cout << "  φ^2 = φ + 1\n";
    cout << "  φ^3 = 2φ + 1\n";
    cout << "  φ^4 = 3φ + 2\n";
    cout << "  φ^5 = 5φ + 3\n";
    cout << "  φ^6 = 8φ + 5\n";
    cout << "  φ^7 = 13φ + 8\n";
    cout << "  φ^8 = 21φ + 13\n\n";
    cout << "  Ang coefficients ay Fibonacci numbers\n";
    cout << "  F_n mod m ay periodic para sa anumang m\n";
    cout << "  Halimbawa, F_n mod 2: 1,1,0,1,1,0,... period 3\n\n";

    // ============================================
    // 7. Golden ratio bilang rotation
    // ============================================
    cout << "--- 7. Golden ratio bilang rotation ---\n\n";
    complex<double> rot1 = exp(complex<double>(0, 2.0 * M_PI / PHI));
    complex<double> rot2 = exp(complex<double>(0, 2.0 * M_PI / (PHI * PHI)));
    
    cout << "  e^(2πi/φ) = " << rot1.real() << " + " << rot1.imag() << "i\n";
    cout << "  e^(2πi/φ²) = " << rot2.real() << " + " << rot2.imag() << "i\n\n";
    cout << "  Kung ang exponent ay nag-a-add ng φ:\n";
    cout << "  e^(2πi(n+φ)/φ) = e^(2πin/φ) × e^(2πi)\n";
    cout << "  = e^(2πin/φ) × 1 = same!\n";
    cout << "  Kaya ang φ ay natural na period sa rotation\n\n";

    // ============================================
    // 8. φ-fractional part bilang map
    // ============================================
    cout << "--- 8. φ-fractional part bilang map ---\n\n";
    cout << "  Map: x → frac(φ × x)\n\n";
    
    double x = 1.0;
    cout << "  x₀ = 1\n";
    for (int i = 1; i <= 15; i++) {
        x = fmod(PHI * x, 1.0);
        cout << "  x" << i << " = " << setw(12) << x << "\n";
    }
    cout << "\n";

    return 0;
}
