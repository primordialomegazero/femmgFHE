// ============================================
// φ-SELF-REF COMPARISON
// Hanapin ang natural na comparison sa φ-space
// Walang explicit na if-then
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-SELF-REF COMPARISON ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Ang φ-power comparison
    // ============================================
    cout << "--- 1. φ-power comparison ---\n\n";
    cout << "  φ^a > φ^b kung at kung a > b\n";
    cout << "  Ang φ-power ay monotonic\n\n";
    
    cout << "  a | b | φ^a - φ^b | Sign\n";
    cout << "  --|---|------------|------\n";
    
    for (int a : {1, 3, 5}) {
        for (int b : {2, 4, 6}) {
            double diff = pow(PHI, a) - pow(PHI, b);
            string sign = diff > 0 ? "+" : (diff < 0 ? "-" : "0");
            
            cout << "  " << setw(1) << a << " | "
                 << setw(1) << b << " | "
                 << setw(12) << diff << " | "
                 << sign << "\n";
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang sign ng φ-power difference
    // ============================================
    cout << "--- 2. Sign ng difference ---\n\n";
    cout << "  φ^a - φ^b = φ^b × (φ^(a-b) - 1)\n";
    cout << "  Ang sign ay depende sa (a-b)\n\n";
    
    cout << "  a-b | φ^(a-b) - 1 | Sign\n";
    cout << "  ----|--------------|------\n";
    
    for (int d : {-3, -2, -1, 0, 1, 2, 3}) {
        double diff = pow(PHI, d) - 1.0;
        string sign = diff > 0 ? "+" : (diff < 0 ? "-" : "0");
        
        cout << "  " << setw(3) << d << " | "
             << setw(13) << diff << " | "
             << sign << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ-complement
    // ============================================
    cout << "--- 3. φ-complement ---\n\n";
    cout << "  φ⁻¹ = 1 - φ⁻² (self-referential!)\n";
    cout << "  φ⁻¹ = " << PHI - 1.0 << "\n";
    cout << "  1 - φ⁻² = " << 1.0 - pow(PHI, -2) << "\n";
    cout << "  Match: " << (abs((PHI-1.0) - (1.0-pow(PHI,-2))) < 1e-12 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 4. Ang threshold detection
    // ============================================
    cout << "--- 4. Threshold detection ---\n\n";
    cout << "  φ^n > threshold kung at kung n > log_φ(threshold)\n";
    cout << "  Natural threshold: φ itself\n\n";
    
    cout << "  n | φ^n | φ^n > φ?\n";
    cout << "  --|-----|----------\n";
    
    for (int n = 0; n <= 5; n++) {
        double phi_n = pow(PHI, n);
        bool greater = phi_n > PHI;
        
        cout << "  " << setw(1) << n << " | "
             << setw(8) << phi_n << " | "
             << (greater ? "YES" : "NO") << "\n";
    }
    cout << "\n";

    // ============================================
    // 5. Ang emergent na comparison
    // ============================================
    cout << "--- 5. Emergent comparison ---\n\n";
    cout << "  φ^i + φ^(i-1) = φ^(i+1)\n";
    cout << "  φ^i - φ^(i-1) = φ^(i-2)\n\n";
    cout << "  Ang sum ng magkasunod ay mas malaki sa bawat isa\n";
    cout << "  Ang difference ay mas maliit sa bawat isa\n\n";
    
    cout << "  i | φ^i + φ^(i-1) | Max | Sum > Max?\n";
    cout << "  --|----------------|-----|------------\n";
    
    for (int i = 1; i <= 8; i++) {
        double sum = pow(PHI, i) + pow(PHI, i-1);
        double max_val = max(pow(PHI, i), pow(PHI, i-1));
        bool greater = sum > max_val;
        
        cout << "  " << setw(1) << i << " | "
             << setw(12) << sum << " | "
             << setw(8) << max_val << " | "
             << (greater ? "YES" : "NO") << "\n";
    }
    cout << "\n";

    // ============================================
    // 6. Ang self-referential na threshold
    // ============================================
    cout << "--- 6. Self-referential threshold ---\n\n";
    cout << "  φ = 1 + φ⁻¹\n";
    cout << "  Kung value > φ, kailangan ng carry\n";
    cout << "  Kung value ≤ φ, walang carry\n\n";
    
    cout << "  Value | > φ? | Carry?\n";
    cout << "  ------|------|--------\n";
    
    for (double val : {0.5, 1.0, 1.5, PHI, 2.0, 2.5, 3.0}) {
        bool greater = val > PHI;
        bool carry = greater;
        
        cout << "  " << setw(5) << val << " | "
             << (greater ? "YES" : "NO ") << " | "
             << (carry ? "YES" : "NO") << "\n";
    }
    cout << "\n";

    return 0;
}
