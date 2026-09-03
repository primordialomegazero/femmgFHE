// ============================================
// φ-EPSILON EXACT
// Hanapin ang eksaktong φ-based na formula
// para sa alternating correction
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

    cout << "=== φ-EPSILON EXACT ===\n\n";
    
    // Para sa malaking n, ε_n × φ^(n+1) × (-1)^n ≈ constant
    cout << "  n | ε_n × φ^(n+1) × (-1)^n | Pattern\n";
    cout << "  --|--------------------------|--------\n";
    
    vector<double> constants;
    for (int n = 7; n <= 20; n++) {
        double F = (double)fib[n];
        double exact_log = log(F) / LN_PHI;
        double approx_log = n - LOG_SQRT5;
        double epsilon = exact_log - approx_log;
        double val = epsilon * pow(PHI, n+1) * pow(-1.0, n);
        constants.push_back(val);
        
        cout << "  " << setw(2) << n << " | "
             << setw(22) << fixed << setprecision(10) << val << " | "
             << setw(6) << (n > 7 ? (abs(val - constants[n-8]) < 0.01 ? "✅" : "❌") : "---") << "\n";
    }
    
    // Average ng constants para sa malaking n
    double sum = 0;
    int count = 0;
    for (int i = 5; i < constants.size(); i++) {
        sum += constants[i];
        count++;
    }
    double C = sum / count;
    
    cout << "\n  Average constant: " << C << "\n";
    cout << "  C × φ = " << C * PHI << "\n";
    cout << "  C × φ² = " << C * PHI * PHI << "\n";
    cout << "  C / φ = " << C / PHI << "\n";
    cout << "  C / φ² = " << C / (PHI * PHI) << "\n";
    cout << "  log_φ(C) = " << log(C) / LN_PHI << "\n";
    cout << "  1/φ³ = " << pow(PHI, -3) << "\n";
    cout << "  1/φ⁴ = " << pow(PHI, -4) << "\n";
    cout << "  1/φ⁵ = " << pow(PHI, -5) << "\n";
    cout << "  1/(φ×√5) = " << 1.0 / (PHI * sqrt(5.0)) << "\n";
    cout << "  1/(φ²×√5) = " << 1.0 / (PHI * PHI * sqrt(5.0)) << "\n";
    cout << "  log_φ(√5)/φ = " << LOG_SQRT5 / PHI << "\n";
    cout << "  log_φ(√5)/φ² = " << LOG_SQRT5 / (PHI * PHI) << "\n";
    cout << "  log_φ(√5)/φ³ = " << LOG_SQRT5 / (PHI * PHI * PHI) << "\n";
    
    cout << "\n=== KEY ===\n";
    cout << "  Kung ang C ay may φ-based na halaga,\n";
    cout << "  ang transition ay fully closed-form\n\n";

    return 0;
}
