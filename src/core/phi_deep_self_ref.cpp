// ============================================
// φ-DEEP SELF-REF
// Mas malalim na self-referential properties
// para sa automatic carry detection
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-DEEP SELF-REF ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Ang φ bilang fixed point ng complement
    // ============================================
    cout << "--- 1. Fixed point ng complement ---\n\n";
    cout << "  φ = 1 + φ⁻¹\n";
    cout << "  φ⁻¹ = 1 - φ⁻²\n";
    cout << "  φ⁻² = φ⁻¹ - φ⁻³\n\n";
    
    cout << "  Identity | Value | Match\n";
    cout << "  ---------|-------|-------\n";
    
    double phi_inv = 1.0 / PHI;
    double phi_inv2 = 1.0 / (PHI * PHI);
    double phi_inv3 = 1.0 / (PHI * PHI * PHI);
    
    cout << "  φ⁻¹ = 1-φ⁻² | " << phi_inv << " = " << 1.0 - phi_inv2 << " | "
         << (abs(phi_inv - (1.0 - phi_inv2)) < 1e-12 ? "✅" : "❌") << "\n";
    cout << "  φ⁻² = φ⁻¹-φ⁻³ | " << phi_inv2 << " = " << phi_inv - phi_inv3 << " | "
         << (abs(phi_inv2 - (phi_inv - phi_inv3)) < 1e-12 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 2. Ang recursive complement
    // ============================================
    cout << "--- 2. Recursive complement ---\n\n";
    cout << "  φ^(-n) = φ^(-(n-1)) - φ^(-(n+1))\n\n";
    
    cout << "  n | φ^(-n) | φ^(-(n-1)) - φ^(-(n+1)) | Match\n";
    cout << "  --|---------|---------------------------|-------\n";
    
    for (int n = 1; n <= 10; n++) {
        double phi_neg_n = pow(PHI, -n);
        double phi_neg_n1 = pow(PHI, -(n-1));
        double phi_neg_np1 = pow(PHI, -(n+1));
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << phi_neg_n << " | "
             << setw(12) << (phi_neg_n1 - phi_neg_np1) << " | "
             << (abs(phi_neg_n - (phi_neg_n1 - phi_neg_np1)) < 1e-10 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang carry bilang complement
    // ============================================
    cout << "--- 3. Carry bilang complement ---\n\n";
    cout << "  Kapag ang value ay lumampas sa φ:\n";
    cout << "  value - φ = value - 1 - φ⁻¹\n";
    cout << "  Ito ay maaaring i-decompose gamit ang complement\n\n";
    
    cout << "  Value | value - φ | Decomposition\n";
    cout << "  ------|-----------|-------------\n";
    
    for (double val : {1.5, PHI, 2.0, 2.618, 3.0, 4.236}) {
        double excess = val - PHI;
        
        cout << "  " << setw(6) << val << " | "
             << setw(10) << excess << " | ";
        
        if (excess < 0) {
            cout << "walang carry\n";
        } else {
            // I-decompose ang excess sa φ-powers
            double remaining = excess;
            for (int p = 3; p >= -5; p--) {
                double phi_p = pow(PHI, p);
                if (remaining >= phi_p - 1e-6) {
                    cout << "φ^" << p << " ";
                    remaining -= phi_p;
                }
            }
            cout << "(rem: " << remaining << ")\n";
        }
    }
    cout << "\n";

    // ============================================
    // 4. Ang self-referential na carry
    // ============================================
    cout << "--- 4. Self-referential carry ---\n\n";
    cout << "  2φ^i = φ^(i+1) + φ^(i-2)\n";
    cout << "  Ito ay maaaring i-split bilang:\n";
    cout << "  φ^i + φ^i = φ^(i+1) + φ^(i-2)\n";
    cout << "  φ^i = φ^(i+1) - φ^i + φ^(i-2)\n";
    cout << "  φ^i = φ^i(φ - 1) + φ^(i-2)\n";
    cout << "  1 = φ - 1 + φ⁻²\n";
    cout << "  1 = φ⁻¹ + φ⁻²\n\n";
    
    cout << "  Check: φ⁻¹ + φ⁻² = " << phi_inv + phi_inv2 << "\n";
    cout << "  Match: " << (abs(phi_inv + phi_inv2 - 1.0) < 1e-12 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 5. Ang emergent na carry decomposition
    // ============================================
    cout << "--- 5. Emergent carry decomposition ---\n\n";
    cout << "  Ang carry ay maaaring i-decompose bilang:\n";
    cout << "  2φ^i = φ^(i+1) + φ^(i-2)\n";
    cout << "       = φ^i × (φ + φ⁻²)\n";
    cout << "       = φ^i × (φ + 1 - φ)\n";
    cout << "       = φ^i × 1\n";
    cout << "       = φ^i\n\n";
    
    cout << "  Wait, ito ay mali...\n";
    cout << "  2φ^i = φ^i + φ^i\n";
    cout << "       = φ^i × (1 + 1)\n";
    cout << "       = φ^i × 2\n";
    cout << "  At: φ^(i+1) + φ^(i-2) = φ^i × (φ + φ⁻²)\n";
    cout << "  φ + φ⁻² = " << PHI + phi_inv2 << "\n";
    cout << "  Match sa 2: " << (abs(PHI + phi_inv2 - 2.0) < 1e-12 ? "✅" : "❌") << "\n\n";

    return 0;
}
