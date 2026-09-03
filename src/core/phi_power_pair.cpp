// ============================================
// φ-POWER PAIR
// (F_n, F_{n-1}) bilang natural na state
// φ-power decomposition na may sync
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-POWER PAIR ===\n\n";
    cout << "  φ¹ = 1×φ + 0\n";
    cout << "  φ² = 1×φ + 1\n";
    cout << "  φ³ = 2×φ + 1\n";
    cout << "  φ⁴ = 3×φ + 2\n";
    cout << "  φ⁵ = 5×φ + 3\n\n";
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    cout << "  n | φ^n | F_n×φ + F_{n-1} | log_φ(φ^n) | n | Diff\n";
    cout << "  --|-----|------------------|------------|---|------\n";
    
    for (int n = 1; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double decomp = fib[n] * PHI + fib[n-1];
        double log_phi_n = log(phi_n) / LN_PHI;
        double diff = log_phi_n - n;
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fixed << setprecision(1) << phi_n << " | "
             << setw(8) << fib[n] << "×φ+" << fib[n-1] << " | "
             << setw(8) << log_phi_n << " | "
             << setw(2) << n << " | "
             << setw(6) << diff << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  log_φ(φ^n) = n — eksakto\n";
    cout << "  φ^n = F_n×φ + F_{n-1} — eksakto\n";
    cout << "  Ang (F_n, F_{n-1}) ay complete normal at log space\n\n";
    
    // ============================================
    // ADDITION SA φ-POWER PAIR
    // ============================================
    cout << "=== ADDITION SA φ-POWER PAIR ===\n\n";
    cout << "  φ^a + φ^b = ?\n\n";
    
    cout << "  a | b | φ^a + φ^b | Decomposition | log_φ | max(a,b)+1 | Diff\n";
    cout << "  --|---|-----------|---------------|-------|------------|------\n";
    
    for (int a = 1; a <= 5; a++) {
        for (int b = a; b <= a+2; b++) {
            double sum = pow(PHI, a) + pow(PHI, b);
            
            // Hanapin ang decomposition ng sum
            double q = floor(sum / PHI);
            double r = fmod(sum, PHI);
            
            double log_sum = log(sum) / LN_PHI;
            double max_plus_1 = max((double)a, (double)b) + 1.0;
            double diff = log_sum - max_plus_1;
            
            cout << "  " << a << " | " << b << " | "
                 << setw(9) << fixed << setprecision(1) << sum << " | "
                 << setw(6) << q << "×φ+" << setprecision(2) << r << " | "
                 << setw(6) << log_sum << " | "
                 << setw(8) << max_plus_1 << " | "
                 << setw(7) << diff << "\n";
        }
    }

    cout << "\n=== KEY ===\n";
    cout << "  Kung ang diff ay 0,\n";
    cout << "  ang φ-power pair addition ay may max+1 na log\n\n";

    return 0;
}
