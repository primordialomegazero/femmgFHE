// ============================================
// φ-EXACT DECOMPOSITION
// Ang 1 + φ^(-d) sa eksaktong φ-terms
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-EXACT DECOMPOSITION ===\n\n";
    
    cout << "  d | 1+φ^(-d) | φ-decomposition | Exact?\n";
    cout << "  --|-----------|------------------|-------\n";
    
    for (int d = 0; d <= 10; d++) {
        double val = 1.0 + pow(PHI, -d);
        
        // Hanapin ang eksaktong φ-decomposition
        // val = a×φ + b kung saan a,b ay integer kung maaari
        double a = floor(val / PHI);
        double b = val - a * PHI;
        
        // Check kung b ay malapit sa Fibonacci/Lucas
        bool is_phi = abs(b - (PHI - 1.0)) < 0.01;  // φ⁻¹
        bool is_zero = abs(b) < 0.01;
        bool is_one = abs(b - 1.0) < 0.01;
        
        cout << "  " << setw(2) << d << " | "
             << setw(9) << fixed << setprecision(4) << val << " | "
             << setw(4) << a << "×φ + " << setw(6) << b << " | "
             << (is_phi ? "φ⁻¹" : is_zero ? "0" : is_one ? "1" : "?") << "\n";
    }
    
    cout << "\n=== PATTERN HUNT ===\n\n";
    cout << "  d=0: 2 = φ + φ⁻¹ = F₂φ + F₁\n";
    cout << "  d=1: φ = φ = F₁φ + F₀\n";
    cout << "  d=2: φ²-φ⁻² = ?\n\n";
    
    cout << "  d | 1+φ^(-d) | φ^k approximation\n";
    cout << "  --|-----------|------------------\n";
    
    for (int d = 0; d <= 5; d++) {
        double val = 1.0 + pow(PHI, -d);
        double k = log(val) / LN_PHI;
        
        cout << "  " << d << " | "
             << setw(9) << fixed << setprecision(4) << val << " | "
             << "φ^" << setw(6) << k << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang 1+φ^(-d) ay may natural na φ-log\n";
    cout << "  na bumababa sa φ-rate\n";
    cout << "  Baka may eksaktong pattern para sa lahat ng d\n\n";

    return 0;
}
