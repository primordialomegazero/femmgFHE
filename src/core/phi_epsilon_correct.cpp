// ============================================
// φ-EPSILON CORRECT
// ε_n = -Σ [(-1)^(kn) × φ^(-2kn)]/k
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double LOG_SQRT5 = log(sqrt(5.0)) / LN_PHI;
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 100; i++) fib.push_back(fib[i-1] + fib[i-2]);

    cout << "=== φ-EPSILON CORRECT ===\n\n";
    cout << "  ε_n = -Σ [(-1)^(kn) × φ^(-2kn)]/k\n\n";
    
    cout << "  n | ε_n actual | ε_n series | Diff\n";
    cout << "  --|------------|-------------|------\n";
    
    for (int n = 3; n <= 20; n++) {
        double F = (double)fib[n];
        double exact_log = log(F) / LN_PHI;
        double approx_log = n - LOG_SQRT5;
        double eps_actual = exact_log - approx_log;
        
        double eps_series = 0;
        for (int k = 1; k <= 5; k++) {
            double sign = pow(-1.0, k * n);
            double term = -sign * pow(PHI, -2.0 * k * n) / k;
            eps_series += term;
        }
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << fixed << setprecision(6) << eps_actual << " | "
             << setw(11) << eps_series << " | "
             << setw(9) << (eps_actual - eps_series) << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Kung ang diff ay maliit,\n";
    cout << "  ang ε_n ay may exact series formula\n\n";

    return 0;
}
