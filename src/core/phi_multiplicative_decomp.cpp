// ============================================
// φ-MULTIPLICATIVE DECOMPOSITION
// Ang integer addition bilang multiplicative factors
// Corrections ay additive sa log space
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-MULTIPLICATIVE DECOMPOSITION ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Ang multiplicative property
    // ============================================
    cout << "--- 1. Multiplicative property ---\n\n";
    cout << "  φ^n + x = φ^n × (1 + x/φ^n)\n";
    cout << "  log_φ(φ^n + x) = n + log_φ(1 + x/φ^n)\n\n";
    
    cout << "  x | ratio (x/φ⁴) | correction\n";
    cout << "  --|---------------|-----------\n";
    
    double phi_4 = pow(PHI, 4);
    for (int x = 1; x <= 10; x++) {
        double ratio = x / phi_4;
        double correction = log(1.0 + ratio) / LN_PHI;
        
        cout << "  " << setw(2) << x << " | "
             << setw(13) << ratio << " | "
             << setw(12) << correction << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Additive corrections sa log space
    // ============================================
    cout << "--- 2. Additive corrections ---\n\n";
    cout << "  Kung ang value ay φ^n × (1+a) × (1+b):\n";
    cout << "  log = n + log_φ(1+a) + log_φ(1+b)\n";
    cout << "  Ang corrections ay additive!\n\n";
    
    // Test: φ⁴ × (1+0.3) × (1+0.5) = φ⁴ × 1.95
    double log_val = 4.0 + log(1.3) / LN_PHI + log(1.5) / LN_PHI;
    double value = pow(PHI, log_val);
    double expected = phi_4 * 1.3 * 1.5;
    
    cout << "  φ⁴ × 1.3 × 1.5 = " << value << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Match: " << (abs(value - expected) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 3. Ang integer bilang multiplicative factors
    // ============================================
    cout << "--- 3. Integer bilang multiplicative factors ---\n\n";
    cout << "  φ⁴ + 5 = φ⁴ × (1 + 5/φ⁴)\n";
    cout << "  5/φ⁴ = " << 5.0/phi_4 << "\n\n";
    
    // Subukan: 1 + 5/φ⁴ = product ng (1 + φ^(-k)) terms
    double target_ratio = 1.0 + 5.0/phi_4;
    cout << "  Target ratio: 1 + 5/φ⁴ = " << target_ratio << "\n\n";
    
    // Hanapin ang φ-power factors
    cout << "  Factor decomposition:\n";
    double remaining = target_ratio;
    vector<int> factors;
    
    for (int k = 1; k <= 10; k++) {
        double factor = 1.0 + pow(PHI, -k);
        if (remaining >= factor - 1e-6) {
            factors.push_back(k);
            remaining /= factor;
            cout << "  × (1 + φ^-" << k << ") = " << factor << "\n";
        }
    }
    
    cout << "  Remaining: " << remaining << "\n";
    cout << "  Product ng factors: ";
    double product = 1.0;
    for (int k : factors) {
        product *= (1.0 + pow(PHI, -k));
    }
    cout << product << "\n";
    cout << "  Target: " << target_ratio << "\n";
    cout << "  Match: " << (abs(product - target_ratio) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 4. FHE test: additive corrections
    // ============================================
    cout << "--- 4. FHE test: additive corrections ---\n\n";
    
    // φ⁴ + 5 = φ⁴ × (1 + 5/φ⁴)
    // correction = log_φ(1 + 5/φ⁴)
    double correction = log(target_ratio) / LN_PHI;
    
    cout << "  Single correction para sa +5: " << correction << "\n";
    cout << "  Ito ay " << (correction < 1.0 ? "single EvalAdd" : "multiple EvalAdds") << "\n";
    cout << "  Level ay hindi maaapektuhan\n\n";
    
    // Alternative: multiple factors
    cout << "  Multiple factors decomposition:\n";
    double total_correction = 0;
    for (int k : factors) {
        double c = log(1.0 + pow(PHI, -k)) / LN_PHI;
        total_correction += c;
        cout << "  + corr(" << k << ") = " << c << "\n";
    }
    cout << "  Total additive correction: " << total_correction << "\n";
    cout << "  Single correction: " << correction << "\n";
    cout << "  Match: " << (abs(total_correction - correction) < 0.01 ? "✅" : "❌") << "\n";

    return 0;
}
