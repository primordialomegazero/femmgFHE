// RULE 110 + FIBONACCI — Emergent Connection
// Rule 110 ay may Fibonacci pattern
// 0,1,1,1,0,1,1,0 — may natural na φ structure

#include <iostream>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 + FIBONACCI\n";
    std::cout << "  Emergent Connection\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

    // Rule 110 binary: 0,1,1,1,0,1,1,0
    // Decimal: 0,1,3,7,6,5,3,0 — may pattern ba?
    
    std::cout << "RULE 110 DECIMAL PATTERN:\n";
    std::cout << "=========================\n\n";
    
    int rule110[] = {0, 1, 1, 1, 0, 1, 1, 0};
    int decimal[] = {0, 1, 3, 7, 6, 5, 3, 0};
    
    for (int i = 0; i < 8; i++) {
        std::cout << "  Input " << i << " (" << ((i>>2)&1) << ((i>>1)&1) << (i&1) 
                  << ") → " << rule110[i] << " (decimal " << decimal[i] << ")\n";
    }
    
    std::cout << "\nFIBONACCI CONNECTION:\n";
    std::cout << "====================\n\n";
    
    // Fibonacci numbers: 0, 1, 1, 2, 3, 5, 8, 13
    long long F[20];
    F[0] = 0; F[1] = 1;
    for (int i = 2; i < 20; i++) F[i] = F[i-1] + F[i-2];
    
    std::cout << "  Fibonacci: ";
    for (int i = 0; i < 10; i++) std::cout << F[i] << " ";
    std::cout << "\n\n";
    
    // Rule 110 output vs Fibonacci mod 2
    std::cout << "  Rule 110 output vs F(n) mod 2:\n";
    for (int i = 0; i < 8; i++) {
        std::cout << "  " << i << " → Rule110=" << rule110[i]
                  << " F(" << i << ") mod 2 = " << (F[i] % 2) << "\n";
    }
    
    std::cout << "\nEMERGENT PATTERN HUNT:\n";
    std::cout << "======================\n\n";
    
    // Ang Rule 110 ay may property:
    // next = (L + C + R) mod 2 kung HINDI (L=1 at C=0 at R=1)
    // At may special case sa (1,0,1)
    
    // Sa φ-space: 0 = ψ, 1 = φ
    // Ang Rule 110 ay: next = φ kung may C=1 o (R=1 at L=0)
    // next = ψ kung C=0 at (R=0 o L=1)
    
    // Subukan: next = C * (1 - R) + (1 - C) * R * (1 - L) + C * (1 - L)
    // Ito ay nagbibigay ng:
    // (0,0,0): 0 ✓
    // (0,0,1): 0*1 + 1*1*1 + 0 = 1 ✓
    // (0,1,0): 1*1 + 0 + 1*1 = 1+1 = 2... modulo 2 = 0 ✗
    
    // Sa BFV, modulo ang natural
    // Kaya next = (L + C + R - LC - CR - LR + LCR) * (1 - L*C*R)
    // + L*C*R*(1-C)
    
    std::cout << "BFV MODULO APPROACH:\n";
    std::cout << "====================\n\n";
    std::cout << "  next = L + C + R - 2*(L*C + C*R + L*R) + 3*L*C*R\n\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int result = L + C + R - 2*(L*C + C*R + L*R) + 3*L*C*R;
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
                          << " mod 2 = " << (result % 2)
                          << " (expected " << expected << ")"
                          << ((result % 2) == expected ? " ✓" : " ✗") << "\n";
            }
        }
    }
    
    return 0;
}
