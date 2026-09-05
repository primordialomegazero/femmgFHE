// ============================================
// φ-ADDITIVE CORRECTION
// Hanapin kung aling decomposition ang may
// additive corrections sa log space
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-ADDITIVE CORRECTION ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Ang correction para sa φ^n + φ^(n-k)
    // ============================================
    cout << "--- 1. Correction: log_φ(1 + φ^(-k)) ---\n\n";
    cout << "  k | φ^(-k) | correction\n";
    cout << "  --|---------|-----------\n";
    
    vector<double> corr_table(15, 0.0);
    for (int k = 1; k <= 14; k++) {
        corr_table[k] = log(1.0 + pow(PHI, -k)) / LN_PHI;
        cout << "  " << setw(2) << k << " | "
             << setw(8) << pow(PHI, -k) << " | "
             << setw(12) << corr_table[k] << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Ang ratio sa pagitan ng corrections
    // ============================================
    cout << "--- 2. Ratio ng corrections ---\n\n";
    cout << "  k | corr(k) | corr(k)/corr(k-1)\n";
    cout << "  --|---------|-------------------\n";
    
    for (int k = 2; k <= 14; k++) {
        double ratio = corr_table[k] / corr_table[k-1];
        cout << "  " << setw(2) << k << " | "
             << setw(9) << corr_table[k] << " | "
             << setw(12) << ratio << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang correction bilang φ-power series
    // ============================================
    cout << "--- 3. Correction bilang φ-power series ---\n\n";
    cout << "  corr(k) = ?\n\n";
    
    // Subukan: corr(k) ≈ φ^(1-k)
    cout << "  k | corr(k) | φ^(1-k) | corr/φ^(1-k)\n";
    cout << "  --|---------|---------|-------------\n";
    
    for (int k = 1; k <= 10; k++) {
        double phi_pow = pow(PHI, 1-k);
        double ratio = corr_table[k] / phi_pow;
        
        cout << "  " << setw(2) << k << " | "
             << setw(9) << corr_table[k] << " | "
             << setw(9) << phi_pow << " | "
             << setw(12) << ratio << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang φ-power decomposition ng corrections
    // ============================================
    cout << "--- 4. φ-power decomposition ng corrections ---\n\n";
    cout << "  corr(k) ≈ Σ φ^(1-j) para sa j=1..k?\n\n";
    
    for (int k = 1; k <= 10; k++) {
        cout << "  corr(" << setw(2) << k << ") = " << setw(10) << corr_table[k] << " | ";
        
        double remaining = corr_table[k];
        vector<int> powers;
        for (int p = 5; p >= -5; p--) {
            double phi_p = pow(PHI, p);
            if (remaining >= phi_p - 1e-6) {
                powers.push_back(p);
                remaining -= phi_p;
            }
        }
        
        for (size_t i = 0; i < powers.size(); i++) {
            if (i > 0) cout << " + ";
            cout << "φ^" << powers[i];
        }
        cout << "\n";
    }
    cout << "\n";

    // ============================================
    // 5. Ang additive property
    // ============================================
    cout << "--- 5. Additive property ---\n\n";
    cout << "  φ^n + φ^(n-k) + φ^(n-j) = ?\n\n";
    
    cout << "  n=4, k=1, j=2:\n";
    {
        double phi_4 = pow(PHI, 4);
        double sum = phi_4 + pow(PHI, 3) + pow(PHI, 2);
        double log_sum = log(sum) / LN_PHI;
        double correction_actual = log_sum - 4;
        
        double corr_k1 = corr_table[1];
        double corr_j2 = corr_table[2];
        double correction_additive = corr_k1 + corr_j2;
        
        cout << "  Actual: log_φ(φ⁴+φ³+φ²) - 4 = " << correction_actual << "\n";
        cout << "  Additive: corr(1)+corr(2) = " << correction_additive << "\n";
        cout << "  Match: " << (abs(correction_actual - correction_additive) < 0.01 ? "✅" : "❌") << "\n\n";
    }
    
    cout << "  n=4, k=1, j=3:\n";
    {
        double phi_4 = pow(PHI, 4);
        double sum = phi_4 + pow(PHI, 3) + pow(PHI, 1);
        double log_sum = log(sum) / LN_PHI;
        double correction_actual = log_sum - 4;
        
        double correction_additive = corr_table[1] + corr_table[3];
        
        cout << "  Actual: " << correction_actual << "\n";
        cout << "  Additive: " << correction_additive << "\n";
        cout << "  Match: " << (abs(correction_actual - correction_additive) < 0.01 ? "✅" : "❌") << "\n\n";
    }

    return 0;
}
