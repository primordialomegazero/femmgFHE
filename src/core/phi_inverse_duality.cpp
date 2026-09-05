// ============================================
// φ-INVERSE DUALITY
// Ang conjugate ψ = -1/φ bilang dual
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double PSI = -1.0 / PHI;
    
    cout << "=== φ-INVERSE DUALITY ===\n\n";
    cout << "  φ = " << PHI << "\n";
    cout << "  ψ = " << PSI << "\n\n";
    cout << "  φ + ψ = " << PHI + PSI << " (dapat 1)\n";
    cout << "  φ × ψ = " << PHI * PSI << " (dapat -1)\n\n";
    
    cout << "  n | φⁿ | ψⁿ | φⁿ + ψⁿ | φⁿ × ψⁿ\n";
    cout << "  --|----|----|---------|---------\n";
    
    for (int n = 1; n <= 10; n++) {
        double phi_n = pow(PHI, n);
        double psi_n = pow(PSI, n);
        
        cout << "  " << setw(2) << n << " | "
             << setw(8) << fixed << setprecision(2) << phi_n << " | "
             << setw(7) << psi_n << " | "
             << setw(7) << (phi_n + psi_n) << " | "
             << setw(7) << (phi_n * psi_n) << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  φⁿ + ψⁿ ay integer (Lucas numbers)\n";
    cout << "  φⁿ × ψⁿ = (-1)ⁿ\n\n";

    return 0;
}
