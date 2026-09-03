// ============================================
// φ-BASIS SPACE
// F = aφ + bφ⁻¹ na may automatic modulo
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-BASIS SPACE ===\n\n";
    
    // ============================================
    // 1. DECOMPOSITION
    // ============================================
    cout << "--- 1. DECOMPOSITION ---\n\n";
    cout << "  F = aφ + bφ⁻¹\n";
    cout << "  φ⁻¹ = φ - 1\n\n";
    
    cout << "  F | a | b | aφ + bφ⁻¹ | Match?\n";
    cout << "  --|---|---|------------|-------\n";
    
    for (double F : {1.0, 2.0, 3.0, 5.0, 7.0, 8.0, 13.0, 21.0, 34.0, 55.0, 100.0}) {
        // a = floor(F/φ)
        double a = floor(F / PHI);
        // b = (F - aφ) / φ⁻¹ = (F - aφ) × φ
        double b = (F - a * PHI) * PHI;
        double recon = a * PHI + b * (PHI - 1.0);
        
        cout << "  " << setw(5) << F << " | "
             << setw(3) << fixed << setprecision(0) << a << " | "
             << setw(5) << b << " | "
             << setw(8) << recon << " | "
             << (abs(recon - F) < 0.01 ? "✅" : "❌") << "\n";
    }

    // ============================================
    // 2. ADDITION SA φ-BASIS
    // ============================================
    cout << "\n--- 2. ADDITION SA φ-BASIS ---\n\n";
    cout << "  (a₁φ + b₁φ⁻¹) + (a₂φ + b₂φ⁻¹) = (a₁+a₂)φ + (b₁+b₂)φ⁻¹\n\n";
    
    cout << "  F₁ | F₂ | F₁+F₂ | (a₁+a₂)φ+(b₁+b₂)φ⁻¹ | Match?\n";
    cout << "  ----|----|-------|---------------------|-------\n";
    
    for (double F1 : {1.0, 3.0, 5.0}) {
        for (double F2 : {2.0, 4.0, 7.0}) {
            double sum = F1 + F2;
            double a1 = floor(F1 / PHI);
            double b1 = (F1 - a1 * PHI) * PHI;
            double a2 = floor(F2 / PHI);
            double b2 = (F2 - a2 * PHI) * PHI;
            double recon = (a1 + a2) * PHI + (b1 + b2) * (PHI - 1.0);
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(5) << sum << " | "
                 << setw(10) << fixed << setprecision(3) << recon << " | "
                 << (abs(recon - sum) < 0.01 ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // 3. AUTOMATIC MODULO
    // ============================================
    cout << "\n--- 3. AUTOMATIC MODULO ---\n\n";
    cout << "  F mod φ = bφ⁻¹\n";
    cout << "  Kapag b ay lumampas ng φ, may carry\n\n";
    
    cout << "  b | bφ⁻¹ | carry?\n";
    cout << "  --|------|-------\n";
    
    for (double b : {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0}) {
        double mod_phi = b * (PHI - 1.0);
        bool carry = b >= PHI;
        
        cout << "  " << setw(4) << b << " | "
             << setw(7) << fixed << setprecision(3) << mod_phi << " | "
             << (carry ? "+1" : " 0") << "\n";
    }

    // ============================================
    // 4. KEY INSIGHT
    // ============================================
    cout << "\n=== KEY ===\n";
    cout << "  Ang φ-basis ay may automatic na modulo\n";
    cout << "  Kung b < φ: walang carry\n";
    cout << "  Kung b ≥ φ: carry +1 sa a\n";
    cout << "  Ito ay natural na periodic reset\n\n";

    return 0;
}
