// RULE 110 DEEPER — Pinaka-emergent na Formula
// Hanapin ang φ-based na formula na may self-referential na collapse

#include <iostream>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 DEEPER\n";
    std::cout << "  Pinaka-emergent na Formula\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

    // Rule 110 truth table sa φ-ψ space
    // 0 = ψ, 1 = φ
    // Pattern: ψ, φ, φ, φ, ψ, φ, φ, ψ
    
    std::cout << "RULE 110 SA φ-ψ SPACE:\n";
    std::cout << "======================\n\n";
    
    // Sa φ-ψ: L, C, R ∈ {ψ, φ}
    // next = ?
    
    // Subukan ang self-referential:
    // next = C * (L + R) - L * R
    
    // Sa binary:
    // (0,0,0): 0 * (0+0) - 0*0 = 0 ✓
    // (0,0,1): 0 * (0+1) - 0*1 = 0 ✗ (dapat 1)
    
    // Subukan: next = C + L*R - 2*L*C*R
    // (0,0,0): 0 ✓
    // (0,0,1): 0 + 0 - 0 = 0 ✗ (dapat 1)
    
    // Subukan: next = (L + C + R) - (L*C + C*R + L*R) + L*C*R
    // Ito ay inclusion-exclusion
    // (0,0,0): 0 ✓
    // (0,0,1): 1 - 0 + 0 = 1 ✓
    // (0,1,0): 1 - 0 + 0 = 1 ✓
    // (0,1,1): 2 - 1 + 0 = 1 ✓
    // (1,0,0): 1 - 0 + 0 = 1 ✗ (dapat 0)
    
    // Ang problema ay (1,0,0) at (1,0,1)
    
    std::cout << "INCLUSION-EXCLUSION PATTERN:\n";
    std::cout << "===========================\n\n";
    std::cout << "  next = (L+C+R) - (LC+CR+LR) + LCR\n\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int sum = L + C + R;
                int pair = L*C + C*R + L*R;
                int triple = L*C*R;
                int result = sum - pair + triple;
                
                int expected;
                switch (L*4 + C*2 + R) {
                    case 0: expected = 0; break;
                    case 1: expected = 1; break;
                    case 2: expected = 1; break;
                    case 3: expected = 1; break;
                    case 4: expected = 0; break;
                    case 5: expected = 1; break;
                    case 6: expected = 1; break;
                    case 7: expected = 0; break;
                }
                
                std::cout << "  (" << L << C << R << ") → " << result
                          << " (expected " << expected << ")"
                          << (result == expected ? " ✓" : " ✗") << "\n";
            }
        }
    }
    
    std::cout << "\nSELF-REFERENTIAL φ APPROACH:\n";
    std::cout << "=============================\n\n";
    std::cout << "  Sa φ-ψ space, subukan ang:\n";
    std::cout << "  next = φ - (L-R)²\n\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                // Sa φ-ψ space
                double l = (L == 0) ? PSI : PHI;
                double c = (C == 0) ? PSI : PHI;
                double r = (R == 0) ? PSI : PHI;
                
                // Subukan ang iba't ibang φ formulas
                double result = c * (l + r) - l * r;
                
                int expected;
                switch (L*4 + C*2 + R) {
                    case 0: expected = 0; break;
                    case 1: expected = 1; break;
                    case 2: expected = 1; break;
                    case 3: expected = 1; break;
                    case 4: expected = 0; break;
                    case 5: expected = 1; break;
                    case 6: expected = 1; break;
                    case 7: expected = 0; break;
                }
                
                double expected_phi = (expected == 0) ? PSI : PHI;
                
                std::cout << "  (" << L << C << R << ") → " << result
                          << " (expected " << expected_phi << ")"
                          << (std::abs(result - expected_phi) < 0.1 ? " ✓" : " ✗") << "\n";
            }
        }
    }
    
    return 0;
}
