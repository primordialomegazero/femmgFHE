// ============================================
// φ-MOD1 RESEARCH
// Mod 1 space bilang natural na FHE encoding
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    
    cout << "=== φ-MOD1 RESEARCH ===\n\n";

    // ============================================
    // 1. MOD 1 DECOMPOSITION
    // ============================================
    cout << "--- 1. MOD 1 DECOMPOSITION ---\n\n";
    cout << "  F = floor(F) + fmod(F, 1)\n";
    cout << "  Ito ay automatic sa integer decomposition\n\n";
    
    cout << "  F | floor(F) | fmod(F,1) | Reconstruct\n";
    cout << "  --|----------|-----------|------------\n";
    
    for (double F : {1.5, 2.3, 3.7, 5.1, 8.9, 13.4, 21.6, 34.2}) {
        double fl = floor(F);
        double fm = fmod(F, 1.0);
        double recon = fl + fm;
        
        cout << "  " << setw(5) << F << " | "
             << setw(6) << fixed << setprecision(0) << fl << " | "
             << setw(7) << fm << " | "
             << setw(7) << recon << "\n";
    }

    // ============================================
    // 2. MOD 1 + ADDITION
    // ============================================
    cout << "\n--- 2. MOD 1 + ADDITION ---\n\n";
    cout << "  (F₁+F₂) mod 1 = (fmod(F₁,1) + fmod(F₂,1)) mod 1\n";
    cout << "  Ito ay automatic kung naka-encode sa mod 1\n\n";
    
    cout << "  F₁ | F₂ | F₁+F₂ | fmod(F₁,1)+fmod(F₂,1) | Match?\n";
    cout << "  ----|----|-------|-----------------------|-------\n";
    
    for (double F1 : {1.2, 2.5, 3.7}) {
        for (double F2 : {1.3, 2.1}) {
            double sum = F1 + F2;
            double mod_sum = fmod(fmod(F1, 1.0) + fmod(F2, 1.0), 1.0);
            double expected = fmod(sum, 1.0);
            
            cout << "  " << setw(4) << F1 << " | "
                 << setw(4) << F2 << " | "
                 << setw(6) << sum << " | "
                 << setw(7) << fixed << setprecision(3) << mod_sum << " | "
                 << (abs(mod_sum - expected) < 0.01 ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // 3. MOD 1 + φ-STRUCTURE
    // ============================================
    cout << "\n--- 3. MOD 1 + φ-STRUCTURE ---\n\n";
    cout << "  Ang fmod(φ, 1) = φ - 1 = φ⁻¹\n";
    cout << "  Ang fmod(φ², 1) = φ² - 2 = φ⁻²\n\n";
    cout << "  n | fmod(φ^n, 1) | Pattern\n";
    cout << "  --|--------------|--------\n";
    
    for (int n = 1; n <= 15; n++) {
        double fm = fmod(pow(PHI, n), 1.0);
        string pattern = (n % 2 == 0) ? "φ⁻ⁿ" : "φ⁻ⁿ";
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << fixed << setprecision(4) << fm << " | "
             << setw(5) << pattern << "\n";
    }

    // ============================================
    // 4. ANG KEY: FLOOR SA MOD 1 SPACE
    // ============================================
    cout << "\n--- 4. FLOOR SA MOD 1 SPACE ---\n\n";
    cout << "  floor(F) = F - fmod(F, 1)\n";
    cout << "  Ito ay automatic sa mod 1 space\n\n";
    
    cout << "  F | fmod(F,1) | F - fmod | floor(F) | Match?\n";
    cout << "  --|-----------|----------|----------|-------\n";
    
    for (double F : {1.5, 2.3, 3.7, 5.1, 8.9}) {
        double fm = fmod(F, 1.0);
        double computed_floor = F - fm;
        double actual_floor = floor(F);
        
        cout << "  " << setw(4) << F << " | "
             << setw(7) << fixed << setprecision(1) << fm << " | "
             << setw(6) << computed_floor << " | "
             << setw(6) << actual_floor << " | "
             << (abs(computed_floor - actual_floor) < 0.01 ? "✅" : "❌") << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang mod 1 space ay may automatic na floor at fmod\n";
    cout << "  Ito ay mas simple kaysa mod φ space\n";
    cout << "  At may natural na φ-structure\n\n";

    return 0;
}
