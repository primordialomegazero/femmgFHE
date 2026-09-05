// ============================================
// φ-EMERGENT ROTATION — Natural Golden Angle
// Ang φ ay may natural na rotation na 2π/φ²
// Ito ba ay may connection sa EvalRotate?
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
    const double GOLDEN_ANGLE = 2.0 * M_PI / (PHI * PHI);

    cout << "=== φ-EMERGENT ROTATION ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Ang golden angle
    // ============================================
    cout << "--- 1. Golden angle ---\n\n";
    cout << "  2π/φ² = " << GOLDEN_ANGLE << " radians\n";
    cout << "         = " << GOLDEN_ANGLE * 180.0 / M_PI << " degrees\n\n";

    // ============================================
    // 2. Ang φ-power bilang rotation
    // ============================================
    cout << "--- 2. φ-power bilang rotation ---\n\n";
    cout << "  e^(2πi×n/φ²) ay natural na φ-rotation\n\n";
    
    cout << "  n | e^(2πi×n/φ²) | Re | Im\n";
    cout << "  --|----------------|----|----\n";
    
    for (int n = 0; n <= 12; n++) {
        complex<double> rot = exp(complex<double>(0, 2.0 * M_PI * n / (PHI * PHI)));
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << rot.real() << " + " << setw(10) << rot.imag() << "i | "
             << setw(6) << rot.real() << " | "
             << setw(6) << rot.imag() << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ-rotation at EvalRotate
    // ============================================
    cout << "--- 3. φ-rotation at EvalRotate ---\n\n";
    cout << "  Ang EvalRotate ay slot-shift sa CKKS\n";
    cout << "  Ang φ-rotation ay phase-shift sa complex plane\n";
    cout << "  May natural na connection ba?\n\n";
    
    cout << "  Ang φ-rotation ay may periodicity:\n";
    cout << "  n mod φ² ≈ n mod 2.618\n\n";
    
    cout << "  n | n mod φ² | rotation angle\n";
    cout << "  --|----------|----------------\n";
    
    for (int n = 0; n <= 15; n++) {
        double mod_phi2 = fmod(n, PHI * PHI);
        double angle = mod_phi2 * 2.0 * M_PI / (PHI * PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(8) << mod_phi2 << " | "
             << setw(10) << angle << " rad\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang Fibonacci at rotation
    // ============================================
    cout << "--- 4. Fibonacci at rotation ---\n\n";
    cout << "  Ang Fibonacci numbers ay may φ-based na rotation\n";
    cout << "  F_n mod φ ay may natural na periodicity\n\n";
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    cout << "  n | F_n | F_n mod φ | rotation\n";
    cout << "  --|-----|-----------|----------\n";
    
    for (int n = 0; n <= 15; n++) {
        double mod_phi = fmod((double)fib[n], PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fib[n] << " | "
             << setw(9) << mod_phi << " | "
             << setw(8) << mod_phi * 2.0 * M_PI / PHI << " rad\n";
    }
    cout << "\n";

    // ============================================
    // 5. Ang natural na rotation index
    // ============================================
    cout << "--- 5. Natural na rotation index ---\n\n";
    cout << "  Ang φ-rotation ay may natural na index:\n";
    cout << "  floor(n/φ²) = rotation count\n";
    cout << "  n mod φ² = rotation offset\n\n";
    
    cout << "  n | floor(n/φ²) | n mod φ²\n";
    cout << "  --|-------------|----------\n";
    
    for (int n = 0; n <= 12; n++) {
        double floor_n = floor(n / (PHI * PHI));
        double mod_n = fmod(n, PHI * PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << floor_n << " | "
             << setw(8) << mod_n << "\n";
    }
    cout << "\n";

    // ============================================
    // 6. Ang emergent na rotation para sa branching
    // ============================================
    cout << "--- 6. Emergent rotation para sa branching ---\n\n";
    cout << "  Ang parity ng floor(n/φ²) ay nagbibigay ng\n";
    cout << "  natural na binary para sa branch selection\n\n";
    
    cout << "  n | floor(n/φ²) | parity | branch\n";
    cout << "  --|-------------|--------|--------\n";
    
    for (int n = 0; n <= 15; n++) {
        double floor_n = floor(n / (PHI * PHI));
        int parity = (int)floor_n % 2;
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << floor_n << " | "
             << setw(6) << parity << " | "
             << (parity == 0 ? "true (×φ)" : "false (÷φ)") << "\n";
    }
    cout << "\n";

    return 0;
}
