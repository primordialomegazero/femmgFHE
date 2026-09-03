// ============================================
// φ-MAX DECOMPOSITION
// Hanapin ang φ-based na max at abs
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-MAX DECOMPOSITION ===\n\n";
    
    // Ang max(a,b) at |a-b| ay maaaring i-decompose sa φ terms
    //
    // max(a,b) = (a+b)/2 + |a-b|/2
    // |a-b| = sqrt((a-b)²)
    //
    // Pero may φ-based na alternative:
    // |a-b| = φ × |floor(a/φ) - floor(b/φ)| + correction
    //
    // Subukan natin:
    
    cout << "  a | b | |a-b| | φ×|floor(a/φ)-floor(b/φ)| | Diff\n";
    cout << "  --|---|------|----------------------------|------\n";
    
    for (int a = 1; a <= 10; a++) {
        for (int b = 1; b <= 5; b++) {
            int abs_diff = abs(a - b);
            int floor_a = (int)floor(a / PHI);
            int floor_b = (int)floor(b / PHI);
            double phi_abs = PHI * abs(floor_a - floor_b);
            double diff = abs_diff - phi_abs;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(2) << b << " | "
                 << setw(4) << abs_diff << " | "
                 << setw(12) << fixed << setprecision(3) << phi_abs << " | "
                 << setw(7) << diff << "\n";
        }
    }
    
    cout << "\n=== ALTERNATIVE: MAX VIA BEATTY ===\n\n";
    cout << "  max(a,b) = floor(max(a,b) × φ) / φ\n";
    cout << "  = floor(max(floor(a×φ), floor(b×φ)) / φ)\n\n";
    
    cout << "  a | b | max(a,b) | floor(max(aφ,bφ))/φ | Diff\n";
    cout << "  --|---|----------|--------------------|------\n";
    
    for (int a : {1, 2, 3, 5, 8, 13}) {
        for (int b : {2, 3, 5, 8}) {
            int m = max(a, b);
            double a_phi = a * PHI;
            double b_phi = b * PHI;
            double max_phi = max(a_phi, b_phi);
            double approx = floor(max_phi) / PHI;
            double diff = m - approx;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(2) << b << " | "
                 << setw(4) << m << " | "
                 << setw(8) << fixed << setprecision(3) << approx << " | "
                 << setw(7) << diff << "\n";
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Hanapin kung aling decomposition ang may\n";
    cout << "  simpleng φ-based na pattern\n\n";

    return 0;
}
