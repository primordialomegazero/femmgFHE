// ============================================
// φ-NUMBER SYSTEM
// Bawat integer bilang binary φ-power sum
// Carry propagation sa φ-space
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-NUMBER SYSTEM ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Ang carry identity
    // ============================================
    cout << "--- 1. Carry identity ---\n\n";
    cout << "  2φ^i = φ^(i+1) + φ^(i-2)\n\n";
    
    cout << "  i | 2φ^i | φ^(i+1) + φ^(i-2) | Match\n";
    cout << "  --|------|---------------------|-------\n";
    
    for (int i = 0; i <= 10; i++) {
        double two_phi_i = 2.0 * pow(PHI, i);
        double carry = pow(PHI, i+1) + pow(PHI, i-2);
        
        cout << "  " << setw(2) << i << " | "
             << setw(8) << two_phi_i << " | "
             << setw(12) << carry << " | "
             << (abs(two_phi_i - carry) < 1e-8 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Binary φ-representation ng integers
    // ============================================
    cout << "--- 2. Binary φ-representation ---\n\n";
    
    auto to_phi_binary = [&](int x) {
        vector<int> bits;
        double remaining = x;
        
        for (int p = 8; p >= -8; p--) {
            double phi_p = pow(PHI, p);
            if (remaining >= phi_p - 1e-6) {
                bits.push_back(p);
                remaining -= phi_p;
            }
        }
        return bits;
    };
    
    cout << "  Integer | φ-power binary\n";
    cout << "  -------|----------------\n";
    
    for (int x = 1; x <= 20; x++) {
        auto bits = to_phi_binary(x);
        cout << "  " << setw(6) << x << " | ";
        for (size_t i = 0; i < bits.size(); i++) {
            if (i > 0) cout << " + ";
            cout << "φ^" << bits[i];
        }
        cout << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Carry propagation
    // ============================================
    cout << "--- 3. Carry propagation ---\n\n";
    cout << "  Kapag may 2φ^i, ito ay nagiging φ^(i+1) + φ^(i-2)\n\n";
    
    cout << "  Example: 2φ³ = φ⁴ + φ¹\n";
    cout << "  2φ³ = " << 2.0 * pow(PHI, 3) << "\n";
    cout << "  φ⁴ + φ¹ = " << pow(PHI, 4) + pow(PHI, 1) << "\n";
    cout << "  Match: " << (abs(2*pow(PHI,3) - (pow(PHI,4)+pow(PHI,1))) < 1e-8 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 4. Ang φ-number system bilang FHE encoding
    // ============================================
    cout << "--- 4. FHE encoding potential ---\n\n";
    cout << "  Kung ang state ay binary φ-power vector:\n";
    cout << "  (c₀, c₁, c₂, ..., cₙ) kung saan cᵢ ∈ {0, 1}\n\n";
    
    cout << "  Addition: component-wise XOR (may carry)\n";
    cout << "  Carry: 2φ^i = φ^(i+1) + φ^(i-2)\n\n";
    
    cout << "  Sa FHE:\n";
    cout << "  - EvalAdd para sa component-wise addition\n";
    cout << "  - EvalRotate para sa carry propagation\n";
    cout << "  - Walang EvalMult, walang bootstrapping\n\n";

    // ============================================
    // 5. Ang carry pattern
    // ============================================
    cout << "--- 5. Carry pattern ---\n\n";
    cout << "  Carry mula sa φ^i papunta sa:\n";
    cout << "  - φ^(i+1) (forward)\n";
    cout << "  - φ^(i-2) (backward)\n\n";
    
    cout << "  i | φ^i | Carry forward | Carry backward\n";
    cout << "  --|-----|----------------|---------------\n";
    
    for (int i = 0; i <= 8; i++) {
        double phi_i = pow(PHI, i);
        double forward = pow(PHI, i+1);
        double backward = pow(PHI, i-2);
        
        cout << "  " << setw(1) << i << " | "
             << setw(8) << phi_i << " | "
             << setw(10) << forward << " | "
             << setw(10) << backward << "\n";
    }
    cout << "\n";

    return 0;
}
