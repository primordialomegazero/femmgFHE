// ============================================
// φ-SELF-CORRECTING GENERAL
// Hanapin ang constant correction para sa
// arbitrary φ-power addition
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-SELF-CORRECTING GENERAL ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. φ^n + φ^(n-k) — may constant correction?
    // ============================================
    cout << "--- 1. φ^n + φ^(n-k) ---\n\n";
    cout << "  k | n=3 | n=5 | n=8 | n=12 | Constant?\n";
    cout << "  --|------|------|------|-------|----------\n";

    for (int k = 1; k <= 6; k++) {
        cout << "  " << setw(1) << k << " |";
        
        vector<double> corrections;
        for (int n : {3, 5, 8, 12}) {
            if (n <= k) continue;
            double phi_n = pow(PHI, n);
            double phi_nk = pow(PHI, n-k);
            double sum = phi_n + phi_nk;
            double log_sum = log(sum) / LN_PHI;
            double correction = log_sum - n;
            
            corrections.push_back(correction);
            cout << " " << setw(6) << correction << " |";
        }
        
        // Check kung constant
        bool is_constant = true;
        for (size_t i = 1; i < corrections.size(); i++) {
            if (abs(corrections[i] - corrections[0]) > 1e-8) {
                is_constant = false;
                break;
            }
        }
        
        cout << " " << (is_constant ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Ang φ^(n-k) bilang fraction ng φ^n
    // ============================================
    cout << "--- 2. φ^(n-k) / φ^n ---\n\n";
    cout << "  k | φ^(-k) | Correction\n";
    cout << "  --|---------|-----------\n";

    for (int k = 1; k <= 10; k++) {
        double phi_neg_k = pow(PHI, -k);
        double correction = log(1.0 + phi_neg_k) / LN_PHI;
        
        cout << "  " << setw(2) << k << " | "
             << setw(8) << phi_neg_k << " | "
             << setw(12) << correction << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang correction bilang φ-power
    // ============================================
    cout << "--- 3. Correction bilang φ-power ---\n\n";
    cout << "  k | correction | φ^(1-k) | correction/φ^(1-k)\n";
    cout << "  --|------------|---------|--------------------\n";

    for (int k = 1; k <= 10; k++) {
        double correction = log(1.0 + pow(PHI, -k)) / LN_PHI;
        double phi_pow = pow(PHI, 1-k);
        double ratio = correction / phi_pow;
        
        cout << "  " << setw(2) << k << " | "
             << setw(11) << correction << " | "
             << setw(9) << phi_pow << " | "
             << setw(12) << ratio << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang self-correcting property sa iba't ibang n
    // ============================================
    cout << "--- 4. Self-correcting sa iba't ibang n ---\n\n";
    cout << "  φ^n + φ^(n-1) = φ^(n+1)\n";
    cout << "  n | φ^n + φ^(n-1) | φ^(n+1) | Match\n";
    cout << "  --|----------------|---------|-------\n";

    for (int n : {1, 2, 3, 5, 8, 13, 21, 34}) {
        double sum = pow(PHI, n) + pow(PHI, n-1);
        double expected = pow(PHI, n+1);
        
        cout << "  " << setw(2) << n << " | "
             << setw(15) << sum << " | "
             << setw(15) << expected << " | "
             << (abs(sum - expected) < 1e-8 ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // 5. Ang generalized self-correcting
    // ============================================
    cout << "--- 5. Generalized self-correcting ---\n\n";
    cout << "  Para sa arbitrary k:\n";
    cout << "  φ^n + φ^(n-k) = φ^(n-k) × (1 + φ^k)\n";
    cout << "  log_φ = n-k + log_φ(1 + φ^k)\n\n";
    
    cout << "  k | log_φ(1+φ^k) | φ^k | 1+φ^k\n";
    cout << "  --|---------------|------|-------\n";
    
    for (int k = 1; k <= 8; k++) {
        double phi_k = pow(PHI, k);
        double one_plus = 1.0 + phi_k;
        double log_val = log(one_plus) / LN_PHI;
        
        cout << "  " << setw(1) << k << " | "
             << setw(14) << log_val << " | "
             << setw(8) << phi_k << " | "
             << setw(10) << one_plus << "\n";
    }
    cout << "\n";

    return 0;
}
