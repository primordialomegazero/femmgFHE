// ============================================
// φ-EMERGENT PROPERTIES
// Hanapin ang property na nagbibigay ng
// additive corrections para sa integer addition
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-EMERGENT PROPERTIES ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Fibonacci representation ng integers
    // ============================================
    cout << "--- 1. Fibonacci representation ng integers ---\n\n";
    cout << "  Integer | Fibonacci decomposition\n";
    cout << "  -------|-----------------------\n";
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    for (int x = 1; x <= 15; x++) {
        cout << "  " << setw(6) << x << " | ";
        
        double remaining = x;
        vector<int> terms;
        for (int i = fib.size()-1; i >= 2; i--) {
            if (remaining >= fib[i]) {
                terms.push_back(i);
                remaining -= fib[i];
            }
        }
        
        for (size_t i = 0; i < terms.size(); i++) {
            if (i > 0) cout << " + ";
            cout << "F_" << terms[i];
        }
        cout << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. φ-power decomposition na may ibang base
    // ============================================
    cout << "--- 2. φ-power decomposition (alternative) ---\n\n";
    cout << "  Integer | φ-power decomposition (greedy)\n";
    cout << "  -------|-------------------------------\n";
    
    for (int x = 1; x <= 20; x++) {
        cout << "  " << setw(6) << x << " | ";
        
        double remaining = x;
        vector<int> powers;
        for (int p = 10; p >= -10; p--) {
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
        cout << "  (error: " << remaining << ")\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ^n + x — iba't ibang x
    // ============================================
    cout << "--- 3. φ^n + x para sa iba't ibang x ---\n\n";
    cout << "  x | log_φ(φ⁴+x) | correction\n";
    cout << "  --|--------------|-----------\n";
    
    double phi_4 = pow(PHI, 4);
    for (int x = 1; x <= 10; x++) {
        double sum = phi_4 + x;
        double log_sum = log(sum) / LN_PHI;
        double correction = log_sum - 4;
        
        cout << "  " << setw(2) << x << " | "
             << setw(12) << log_sum << " | "
             << setw(12) << correction << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang correction para sa iba't ibang φ-powers
    // ============================================
    cout << "--- 4. Correction para sa φ^n + 1 ---\n\n";
    cout << "  n | φ^n + 1 | correction\n";
    cout << "  --|---------|-----------\n";
    
    for (int n = 1; n <= 10; n++) {
        double phi_n = pow(PHI, n);
        double sum = phi_n + 1.0;
        double log_sum = log(sum) / LN_PHI;
        double correction = log_sum - n;
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << sum << " | "
             << setw(12) << correction << "\n";
    }
    cout << "\n";

    // ============================================
    // 5. Ang multiplicative property
    // ============================================
    cout << "--- 5. Multiplicative property ---\n\n";
    cout << "  (φ^n + x) / φ^n = 1 + x/φ^n\n";
    cout << "  log_φ(1 + x/φ^n) = correction\n\n";
    
    cout << "  x | correction (n=4) | φ^(-n) × x\n";
    cout << "  --|-------------------|------------\n";
    
    for (int x = 1; x <= 10; x++) {
        double correction = log(1.0 + x / phi_4) / LN_PHI;
        double approx = x / phi_4 / LN_PHI;
        
        cout << "  " << setw(2) << x << " | "
             << setw(18) << correction << " | "
             << setw(12) << approx << "\n";
    }
    cout << "\n";

    // ============================================
    // 6. Ang Lucas/Fibonacci correction
    // ============================================
    cout << "--- 6. Lucas/Fibonacci correction ---\n\n";
    cout << "  Lucas L_n = φ^n + φ^(-n)\n";
    cout << "  n | L_n | correction para sa +L_n\n";
    cout << "  --|-----|--------------------------\n";
    
    for (int n = 1; n <= 8; n++) {
        double L_n = pow(PHI, n) + pow(PHI, -n);
        double sum = phi_4 + L_n;
        double log_sum = log(sum) / LN_PHI;
        double correction = log_sum - 4;
        
        cout << "  " << setw(1) << n << " | "
             << setw(12) << L_n << " | "
             << setw(18) << correction << "\n";
    }
    cout << "\n";

    return 0;
}
