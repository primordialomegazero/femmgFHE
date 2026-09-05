// ============================================
// φ-ARBITRARY ADD EMERGENT
// Hanapin ang natural na decomposition
// para sa constant correction ng arbitrary addition
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-ARBITRARY ADD EMERGENT ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. φ^n + x para sa iba't ibang x
    // ============================================
    cout << "--- 1. φ^n + x — correction pattern ---\n\n";
    cout << "  x | φ⁴ + x | log_φ | correction\n";
    cout << "  --|---------|-------|-----------\n";
    
    double phi_4 = pow(PHI, 4);
    for (int x = 1; x <= 15; x++) {
        double sum = phi_4 + x;
        double log_sum = log(sum) / LN_PHI;
        double correction = log_sum - 4;
        
        cout << "  " << setw(2) << x << " | "
             << setw(8) << sum << " | "
             << setw(8) << log_sum << " | "
             << setw(10) << correction << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Ang correction bilang φ-power decomposition
    // ============================================
    cout << "--- 2. Correction bilang φ-power ---\n\n";
    cout << "  x | correction | φ-decomposition\n";
    cout << "  --|------------|----------------\n";
    
    for (int x : {1, 2, 3, 5, 8, 13}) {
        double sum = phi_4 + x;
        double log_sum = log(sum) / LN_PHI;
        double correction = log_sum - 4;
        
        cout << "  " << setw(2) << x << " | "
             << setw(11) << correction << " | ";
        
        double remaining = correction;
        for (int p = 5; p >= -8; p--) {
            double phi_p = pow(PHI, p);
            if (remaining >= phi_p - 1e-6) {
                cout << "φ^" << p << " ";
                remaining -= phi_p;
            }
        }
        cout << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang special na values
    // ============================================
    cout << "--- 3. Special na values ---\n\n";
    cout << "  x | φ⁴ + x | = φ^? | Correction\n";
    cout << "  --|---------|-------|-----------\n";
    
    // Hanapin kung aling x ang nagbibigay ng integer correction
    for (int x = 1; x <= 100; x++) {
        double sum = phi_4 + x;
        double log_sum = log(sum) / LN_PHI;
        double correction = log_sum - 4;
        
        // Check kung ang correction ay malapit sa integer
        double nearest_int = round(correction);
        if (abs(correction - nearest_int) < 0.01) {
            cout << "  " << setw(3) << x << " | "
                 << setw(8) << sum << " | "
                 << "φ^" << 4 + nearest_int << " | "
                 << setw(10) << correction << " ★\n";
        }
    }
    cout << "\n";

    // ============================================
    // 4. Ang Fibonacci na special values
    // ============================================
    cout << "--- 4. Fibonacci na special values ---\n\n";
    cout << "  x | φ⁴ + x | φ-power?\n";
    cout << "  --|---------|----------\n";
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 15; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    for (int i = 0; i < fib.size(); i++) {
        double x = fib[i];
        double sum = phi_4 + x;
        double log_sum = log(sum) / LN_PHI;
        
        cout << "  F_" << i << "=" << setw(4) << x << " | "
             << setw(8) << sum << " | "
             << setw(10) << log_sum << "\n";
    }
    cout << "\n";

    return 0;
}
