// ============================================
// φ-BEATTY ALIGNMENT
// Natural na alignment sa pamamagitan ng Beatty
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-BEATTY ALIGNMENT ===\n\n";

    // ============================================
    // 1. ANG BEATTY PARTITION
    // ============================================
    cout << "--- 1. BEATTY PARTITION ---\n\n";
    cout << "  floor(nφ) at floor(nφ²) ay nagpa-partition\n";
    cout << "  ng positive integers nang walang overlap\n\n";
    
    cout << "  n | floor(nφ) | floor(nφ²)\n";
    cout << "  --|-----------|-----------\n";
    
    for (int n = 1; n <= 10; n++) {
        cout << "  " << setw(2) << n << " | "
             << setw(9) << (int)floor(n * PHI) << " | "
             << setw(9) << (int)floor(n * PHI * PHI) << "\n";
    }

    // ============================================
    // 2. BEATTY-BASED ALIGNMENT
    // ============================================
    cout << "\n--- 2. BEATTY-BASED ALIGNMENT ---\n\n";
    cout << "  Kung k₁ at k₂ ay nasa Beatty sequences,\n";
    cout << "  ang alignment ay natural\n\n";
    
    cout << "  a | b | floor(aφ) - floor(bφ²) | Pattern\n";
    cout << "  --|---|------------------------|--------\n";
    
    for (int a = 1; a <= 5; a++) {
        for (int b = 1; b <= 3; b++) {
            int diff = (int)floor(a * PHI) - (int)floor(b * PHI * PHI);
            
            cout << "  " << a << " | " << b << " | "
                 << setw(8) << diff << " | "
                 << setw(6) << (diff >= 0 ? "≥0" : "<0") << "\n";
        }
    }

    // ============================================
    // 3. ANG KEY: AUTOMATIC ALIGNMENT
    // ============================================
    cout << "\n--- 3. AUTOMATIC ALIGNMENT ---\n\n";
    cout << "  Ang Beatty partition ay nagbibigay ng natural\n";
    cout << "  na paghihiwalay ng values sa dalawang grupo\n\n";
    
    cout << "  Grupo A (floor nφ): maliliit na values\n";
    cout << "  Grupo B (floor nφ²): malalaking values\n\n";

    // ============================================
    // 4. ANG MAS SIMPLENG APPROACH
    // ============================================
    cout << "--- 4. MAS SIMPLENG APPROACH ---\n\n";
    cout << "  Sa halip na i-align, gamitin ang Beatty\n";
    cout << "  para sa AUTOMATIC na modulo\n\n";
    
    cout << "  F | floor(F/φ) | F mod φ | Grupo\n";
    cout << "  --|------------|---------|-------\n";
    
    for (double F : {2.0, 3.0, 5.0, 7.0, 8.0, 13.0, 21.0, 34.0, 55.0, 100.0}) {
        int q = (int)floor(F / PHI);
        double mod_val = fmod(F, PHI);
        bool in_A = (q % 2 == 0);
        
        cout << "  " << setw(5) << F << " | "
             << setw(6) << q << " | "
             << setw(7) << fixed << setprecision(3) << mod_val << " | "
             << (in_A ? "A" : "B") << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang Beatty partition ay nagbibigay ng natural\n";
    cout << "  na alignment at modulo na walang comparison\n\n";

    return 0;
}
