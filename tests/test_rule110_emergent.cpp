// RULE 110 EMERGENT — Natural Arithmetic Formula
// Hanapin ang simpleng formula na nagbibigay ng 8/8

#include <iostream>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 EMERGENT\n";
    std::cout << "  Natural Arithmetic Formula\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

    std::cout << "RULE 110 TRUTH TABLE (φ-ψ space):\n";
    std::cout << "==================================\n\n";
    std::cout << "  L C R | Next | Decimal | φ-space\n";
    std::cout << "  ------|------|---------|---------\n";

    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int decimal = L * 4 + C * 2 + R;
                int next;
                switch (decimal) {
                    case 0: next = 0; break;
                    case 1: next = 1; break;
                    case 2: next = 1; break;
                    case 3: next = 1; break;
                    case 4: next = 0; break;
                    case 5: next = 1; break;
                    case 6: next = 1; break;
                    case 7: next = 0; break;
                }
                
                // Sa φ-space: 0 → ψ, 1 → φ
                double phi_val = (next == 0) ? PSI : PHI;
                
                std::cout << "  " << L << " " << C << " " << R << " |  " << next
                          << "    | " << decimal << "       | " << phi_val << "\n";
            }
        }
    }

    std::cout << "\nNATURAL PATTERN HUNT:\n";
    std::cout << "====================\n\n";
    
    // Ang Rule 110 ay may pattern:
    // 0,1,1,1,0,1,1,0
    // Sa φ-space: ψ,φ,φ,φ,ψ,φ,φ,ψ
    
    // Subukan ang arithmetic formula:
    // next = L + C + R - 2 (kung L+C+R > 2)
    // next = L + C + R (kung L+C+R ≤ 2)
    
    std::cout << "  Arithmetic sums:\n";
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int sum = L + C + R;
                int next;
                switch (L * 4 + C * 2 + R) {
                    case 0: next = 0; break;
                    case 1: next = 1; break;
                    case 2: next = 1; break;
                    case 3: next = 1; break;
                    case 4: next = 0; break;
                    case 5: next = 1; break;
                    case 6: next = 1; break;
                    case 7: next = 0; break;
                }
                std::cout << "  " << L << C << R << " → sum=" << sum
                          << " next=" << next << "\n";
            }
        }
    }

    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  sum=0 → next=0\n";
    std::cout << "  sum=1 → next=1\n";
    std::cout << "  sum=2 → next=1 (L=0,C=1,R=1) o 0 (L=1,C=0,R=1) o 1 (L=1,C=1,R=0)\n";
    std::cout << "  sum=3 → next=0\n\n";
    std::cout << "  Pattern: next = 1 kung (sum=1) OR (sum=2 AND C=1)\n";
    std::cout << "  next = 0 kung sum=0 OR sum=3 OR (sum=2 AND L=1 AND R=1)\n\n";

    return 0;
}
