// ============================================
// φ-RATIO SPACE
// F = r × φ^k kung saan r ∈ [1, φ)
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-RATIO SPACE ===\n\n";
    
    // ============================================
    // 1. DECOMPOSITION
    // ============================================
    cout << "--- 1. DECOMPOSITION ---\n\n";
    cout << "  F = r × φ^k\n";
    cout << "  r ∈ [1, φ), k = floor(log_φ(F))\n\n";
    
    cout << "  F | r | k | r×φ^k | Match?\n";
    cout << "  --|---|----|-------|-------\n";
    
    for (double F : {1.0, 2.0, 3.0, 5.0, 7.0, 8.0, 13.0, 21.0, 34.0, 55.0, 100.0}) {
        double k = floor(log(F) / LN_PHI);
        double r = F / pow(PHI, k);
        double recon = r * pow(PHI, k);
        
        cout << "  " << setw(5) << F << " | "
             << setw(5) << fixed << setprecision(3) << r << " | "
             << setw(3) << (int)k << " | "
             << setw(8) << recon << " | "
             << (abs(recon - F) < 0.01 ? "✅" : "❌") << "\n";
    }

    // ============================================
    // 2. MULTIPLICATION SA RATIO SPACE
    // ============================================
    cout << "\n--- 2. MULTIPLICATION ---\n\n";
    cout << "  F₁ × F₂ = (r₁×r₂) × φ^(k₁+k₂)\n";
    cout << "  Ang r₁×r₂ ay nasa [1, φ²) — kailangan ng renormalize\n\n";
    
    cout << "  F₁ | F₂ | F₁×F₂ | r₁×r₂ | φ^(k₁+k₂) | Match?\n";
    cout << "  ----|----|-------|-------|-----------|-------\n";
    
    for (double F1 : {2.0, 3.0, 5.0}) {
        for (double F2 : {2.0, 4.0}) {
            double prod = F1 * F2;
            double k1 = floor(log(F1) / LN_PHI);
            double r1 = F1 / pow(PHI, k1);
            double k2 = floor(log(F2) / LN_PHI);
            double r2 = F2 / pow(PHI, k2);
            double recon = (r1 * r2) * pow(PHI, k1 + k2);
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(5) << prod << " | "
                 << setw(6) << fixed << setprecision(2) << (r1*r2) << " | "
                 << setw(6) << (int)(k1+k2) << " | "
                 << (abs(recon - prod) < 0.1 ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // 3. ADDITION SA RATIO SPACE
    // ============================================
    cout << "\n--- 3. ADDITION ---\n\n";
    cout << "  F₁ + F₂ = r₁φ^k₁ + r₂φ^k₂\n";
    cout << "  Kung k₁ = k₂: (r₁+r₂) × φ^k\n";
    cout << "  Kung k₁ ≠ k₂: kailangan ng alignment\n\n";
    
    cout << "  F₁ | F₂ | F₁+F₂ | k₁ | k₂ | Match?\n";
    cout << "  ----|----|-------|----|----|-------\n";
    
    for (double F1 : {2.0, 5.0, 13.0}) {
        for (double F2 : {3.0, 8.0}) {
            double sum = F1 + F2;
            double k1 = floor(log(F1) / LN_PHI);
            double k2 = floor(log(F2) / LN_PHI);
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(5) << sum << " | "
                 << setw(3) << (int)k1 << " | "
                 << setw(3) << (int)k2 << " | "
                 << (k1 == k2 ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang ratio space ay natural para sa multiplication\n";
    cout << "  Ang addition ay nangangailangan ng alignment\n";
    cout << "  Ang r ∈ [1, φ) ay bounded\n\n";

    return 0;
}
