// ============================================
// φ-MAX
// φ-based na max na walang comparison
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    
    cout << "=== φ-MAX ===\n\n";
    cout << "  Hanapin ang φ-based na formula para sa max\n\n";
    
    // Test kung ang max ay may simpleng φ expression
    cout << "  a | b | max(a,b) | φ-based approximation | Diff\n";
    cout << "  --|---|----------|----------------------|------\n";
    
    for (int a : {1, 2, 3, 5, 8, 13}) {
        for (int b : {2, 3, 5, 8}) {
            int m = max(a, b);
            
            // φ-based approximation: φ × min(a,b) + |a-b| × φ⁻¹
            double approx = PHI * min(a, b) + abs(a - b) * (PHI - 1.0);
            double diff = m - approx;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(2) << b << " | "
                 << setw(4) << m << " | "
                 << setw(8) << fixed << setprecision(3) << approx << " | "
                 << setw(8) << diff << "\n";
        }
    }
    
    cout << "\n=== SIMPLER: MAX VIA PHI-POWER ===\n\n";
    cout << "  φ^a + φ^b ≈ φ^max(a,b) × (1 + φ^(-|a-b|))\n\n";
    cout << "  |a-b| | 1 + φ^(-|a-b|) | φ^(max(a,b)+1)\n";
    cout << "  ------|-----------------|------------------\n";
    
    for (int d = 0; d <= 5; d++) {
        double factor = 1.0 + pow(PHI, -d);
        cout << "  " << setw(3) << d << " | "
             << setw(12) << fixed << setprecision(4) << factor << " | "
             << setw(12) << pow(PHI, d+1) << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang max ay maaaring i-approximate ng φ-based formula\n";
    cout << "  na walang comparison\n\n";

    return 0;
}
