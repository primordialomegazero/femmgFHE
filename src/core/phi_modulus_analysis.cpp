// ============================================
// φ-MODULUS ANALYSIS — Modulus at φ
// May φ-structure ba ang CKKS modulus?
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-MODULUS ANALYSIS ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Ang ring dimension at φ-powers
    // ============================================
    cout << "--- 1. Ring dimension at φ-powers ---\n\n";
    cout << "  Ring dim = 16384 = 2^14\n\n";
    
    cout << "  Pinakamalapit na φ-power sa 16384:\n";
    for (int n = 0; n <= 30; n++) {
        double phi_n = pow(PHI, n);
        if (abs(phi_n - 16384) < 1000) {
            cout << "    φ^" << n << " = " << phi_n 
                 << " (diff: " << phi_n - 16384 << ")\n";
        }
    }
    cout << "\n";

    // ============================================
    // 2. Ang modulus at φ-powers
    // ============================================
    cout << "--- 2. Modulus at φ-powers ---\n\n";
    cout << "  Modulus ≈ 3.7 × 10^41\n\n";
    
    double log10_mod = 41.568;
    double log_phi_mod = log10_mod / log10(PHI);
    
    cout << "  log10(modulus) = " << log10_mod << "\n";
    cout << "  log_φ(modulus) = " << log_phi_mod << "\n";
    cout << "  Pinakamalapit na integer: " << round(log_phi_mod) << "\n\n";

    // ============================================
    // 3. Ang scaling factor at φ
    // ============================================
    cout << "--- 3. Scaling factor at φ ---\n\n";
    
    double scaling = pow(2.0, 59);
    double log_phi_scaling = log(scaling) / LN_PHI;
    
    cout << "  Scaling = 2^59 = " << scaling << "\n";
    cout << "  log_φ(scaling) = " << log_phi_scaling << "\n";
    cout << "  Pinakamalapit na integer: " << round(log_phi_scaling) << "\n\n";

    // ============================================
    // 4. Ang φ at ang slot count
    // ============================================
    cout << "--- 4. φ at slot count ---\n\n";
    cout << "  Batch size = 8\n";
    cout << "  φ^? = 8 → log_φ(8) = " << log(8.0) / LN_PHI << "\n\n";
    
    cout << "  φ^4 = " << pow(PHI, 4) << " ≈ 6.854\n";
    cout << "  φ^5 = " << pow(PHI, 5) << " ≈ 11.09\n";
    cout << "  Ang 8 ay nasa pagitan ng φ⁴ at φ⁵\n\n";

    // ============================================
    // 5. Ang natural na φ-based na parameters
    // ============================================
    cout << "--- 5. Natural na φ-based na parameters ---\n\n";
    cout << "  Kung gagamit tayo ng φ-based na CKKS:\n";
    cout << "  Ring dim = φ^20 ≈ " << pow(PHI, 20) << "\n";
    cout << "  Scaling = φ^85 ≈ " << pow(PHI, 85) << "\n";
    cout << "  Batch = φ^4 ≈ " << pow(PHI, 4) << "\n\n";

    return 0;
}
