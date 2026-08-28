// PHI COLLAPSE — Natural na Collapse ng Rule 110
// Ang φ ay may hidden properties na pwede nating gamitin
// Para i-collapse ang Rule 110 sa simpleng formula

#include <iostream>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI COLLAPSE\n";
    std::cout << "  Natural Rule 110 Formula\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

    // Rule 110 pattern: 0,1,1,1,0,1,1,0
    // Sa φ-space: ψ,φ,φ,φ,ψ,φ,φ,ψ
    
    std::cout << "RULE 110 PATTERN ANALYSIS:\n";
    std::cout << "==========================\n\n";
    
    // Ang pattern ay may symmetry:
    // 0 1 1 1 | 0 1 1 0
    // Unang kalahati: 0,1,1,1 (L=0)
    // Pangalawang kalahati: 0,1,1,0 (L=1)
    
    // Para sa L=0: next = C OR R
    // Para sa L=1: next = C XOR R (?) 
    //   (1,0,0) → 0 = 0 XOR 0 ✓
    //   (1,0,1) → 1 = 0 XOR 1 ✓
    //   (1,1,0) → 1 = 1 XOR 0 ✓
    //   (1,1,1) → 0 = 1 XOR 1 ✓
    
    std::cout << "EMERGENT XOR-OR PATTERN:\n";
    std::cout << "========================\n\n";
    std::cout << "  L=0: next = C OR R\n";
    std::cout << "  L=1: next = C XOR R\n\n";
    
    // Sa φ-space:
    // OR(C,R) = C + R - C*R
    // XOR(C,R) = C + R - 2*C*R
    
    // next = (1-L)*(C + R - C*R) + L*(C + R - 2*C*R)
    // = C + R - C*R - L*C*R
    
    // Test: next = C + R - C*R - L*C*R
    std::cout << "SELF-REFERENTIAL FORMULA:\n";
    std::cout << "=========================\n\n";
    std::cout << "  next = C + R - C*R - L*C*R\n\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int result = C + R - C*R - L*C*R;
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
    
    std::cout << "\nφ-SPACE VERSION:\n";
    std::cout << "================\n\n";
    std::cout << "  Sa φ-ψ: next = C + R - C*R - L*C*R\n";
    std::cout << "  Kung L,C,R ∈ {ψ, φ}:\n\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                double l = (L == 0) ? PSI : PHI;
                double c = (C == 0) ? PSI : PHI;
                double r = (R == 0) ? PSI : PHI;
                
                double result = c + r - c*r - l*c*r;
                
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
                bool match = std::abs(result - expected_phi) < 0.01;
                
                std::cout << "  (" << L << C << R << ") → " << result
                          << " (expected " << expected_phi << ")"
                          << (match ? " ✓" : " ✗") << "\n";
            }
        }
    }
    
    return 0;
}
