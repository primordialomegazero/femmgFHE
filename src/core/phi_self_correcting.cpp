// ============================================
// φ-SELF-CORRECTING — Verification
// 1 + φ⁻¹ = φ — ang automatic na correction
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-SELF-CORRECTING ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Ang basic property
    // ============================================
    cout << "--- 1. Basic property ---\n\n";
    cout << "  1 + φ⁻¹ = " << 1.0 + 1.0/PHI << " = φ = " << PHI << "\n";
    cout << "  Match: " << (abs(1.0 + 1.0/PHI - PHI) < 1e-12 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 2. Ang correction sa log space
    // ============================================
    cout << "--- 2. Correction sa log space ---\n\n";
    cout << "  log_φ(1 + φ⁻¹) = " << log(1.0 + 1.0/PHI) / LN_PHI << "\n";
    cout << "  Dapat: 1\n\n";

    // ============================================
    // 3. Generalization: φ^n + φ^(n-1)
    // ============================================
    cout << "--- 3. Generalization ---\n\n";
    cout << "  n | φ^n + φ^(n-1) | φ^(n+1) | Match\n";
    cout << "  --|----------------|---------|-------\n";
    
    for (int n = 0; n <= 10; n++) {
        double phi_n = pow(PHI, n);
        double phi_n1 = pow(PHI, n-1);
        double phi_np1 = pow(PHI, n+1);
        
        cout << "  " << setw(2) << n << " | "
             << setw(14) << phi_n + phi_n1 << " | "
             << setw(14) << phi_np1 << " | "
             << (abs(phi_n + phi_n1 - phi_np1) < 1e-10 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang correction ay CONSTANT
    // ============================================
    cout << "--- 4. Constant correction ---\n\n";
    cout << "  log_φ(1 + φ⁻¹) = 1 para sa LAHAT ng n\n";
    cout << "  Ito ay HINDI state-dependent\n\n";
    
    cout << "  n | log_φ(φ^n + φ^(n-1)) | n+1 | Match\n";
    cout << "  --|------------------------|-----|-------\n";
    
    for (int n = 1; n <= 10; n++) {
        double sum = pow(PHI, n) + pow(PHI, n-1);
        double log_sum = log(sum) / LN_PHI;
        
        cout << "  " << setw(2) << n << " | "
             << setw(22) << log_sum << " | "
             << setw(3) << n+1 << " | "
             << (abs(log_sum - (n+1)) < 1e-10 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 5. Decomposition ng arbitrary addition
    // ============================================
    cout << "--- 5. Decomposition ng arbitrary addition ---\n\n";
    cout << "  +5 ay maaaring i-decompose sa φ-powers:\n";
    
    // Hanapin ang φ-decomposition ng 5
    double target = 5.0;
    vector<int> powers;
    double remaining = target;
    
    for (int p = 5; p >= -5; p--) {
        double phi_p = pow(PHI, p);
        if (remaining >= phi_p - 1e-6) {
            powers.push_back(p);
            remaining -= phi_p;
        }
    }
    
    cout << "  5 ≈ ";
    for (size_t i = 0; i < powers.size(); i++) {
        if (i > 0) cout << " + ";
        cout << "φ^" << powers[i];
    }
    cout << "\n";
    cout << "  Exact: 5 = " << target << "\n";
    cout << "  Approximation: ";
    double approx = 0;
    for (int p : powers) {
        approx += pow(PHI, p);
    }
    cout << approx << "\n";
    cout << "  Error: " << abs(approx - target) << "\n\n";

    return 0;
}
