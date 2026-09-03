// ============================================
// φ-EPSILON FINAL
// I-verify ang factor 2 sa series
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

    cout << "=== φ-EPSILON FINAL ===\n\n";
    cout << "  Test kung ang ε_n = 2 × series\n\n";
    
    cout << "  n | ε_n actual | 2×series | Diff\n";
    cout << "  --|------------|----------|------\n";
    
    for (int n = 3; n <= 15; n++) {
        double F = (double)fib[n];
        double exact_log = log(F) / LN_PHI;
        double approx_log = n - LOG_SQRT5;
        double eps_actual = exact_log - approx_log;
        
        double eps_series = 0;
        for (int k = 1; k <= 10; k++) {
            double sign = pow(-1.0, k * n);
            double term = -sign * pow(PHI, -2.0 * k * n) / k;
            eps_series += term;
        }
        
        double eps_2x = 2.0 * eps_series;
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << fixed << setprecision(6) << eps_actual << " | "
             << setw(10) << eps_2x << " | "
             << setw(9) << (eps_actual - eps_2x) << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Kung ang 2×series ay match,\n";
    cout << "  ang ε_n ay may exact closed form\n\n";

    return 0;
}
