// ============================================
// φ-EPSILON FORMULA
// Hanapin ang eksaktong formula para sa ε_n
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

    cout << "=== φ-EPSILON FORMULA ===\n\n";
    cout << "  n | ε_n | ε_n × φ^n | ε_n × φ^(n+1) | Pattern?\n";
    cout << "  --|-----|-----------|----------------|--------\n";

    for (int n = 3; n <= 20; n++) {
        double F = (double)fib[n];
        double exact_log = log(F) / LN_PHI;
        double approx_log = n - LOG_SQRT5;
        double epsilon = exact_log - approx_log;
        double eps_phi_n = epsilon * pow(PHI, n);
        double eps_phi_n1 = epsilon * pow(PHI, n+1);
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << fixed << setprecision(6) << epsilon << " | "
             << setw(12) << eps_phi_n << " | "
             << setw(14) << eps_phi_n1 << " | "
             << setw(6) << (abs(eps_phi_n - eps_phi_n1) < 0.1 ? "✅" : "❌") << "\n";
    }

    cout << "\n=== HYPOTHESIS ===\n";
    cout << "  Kung ε_n × φ^n ≈ constant,\n";
    cout << "  ang formula ay ε_n ≈ C × φ^(-n)\n";
    cout << "  kung saan C ay isang constant\n\n";

    // Hanapin ang constant
    double sum = 0;
    int count = 0;
    for (int n = 5; n <= 20; n++) {
        double F = (double)fib[n];
        double exact_log = log(F) / LN_PHI;
        double approx_log = n - LOG_SQRT5;
        double epsilon = exact_log - approx_log;
        double C_est = epsilon * pow(PHI, n);
        sum += C_est;
        count++;
    }
    double C_avg = sum / count;
    
    cout << "  Average C: " << C_avg << "\n";
    cout << "  φ^(-1) = " << PHI - 1.0 << "\n";
    cout << "  φ^(-2) = " << 2.0 - PHI << "\n";
    cout << "  log_φ(φ) = 1\n\n";

    // Test ang hypothesis
    cout << "=== TEST: ε_n ≈ C × φ^(-n) ===\n\n";
    cout << "  n | ε_n actual | ε_n approx | Diff\n";
    cout << "  --|------------|------------|------\n";
    for (int n = 5; n <= 15; n++) {
        double F = (double)fib[n];
        double exact_log = log(F) / LN_PHI;
        double approx_log = n - LOG_SQRT5;
        double eps_actual = exact_log - approx_log;
        double eps_approx = C_avg * pow(PHI, -n);
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << fixed << setprecision(6) << eps_actual << " | "
             << setw(10) << eps_approx << " | "
             << setw(9) << (eps_actual - eps_approx) << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Kung ang diff ay maliit,\n";
    cout << "  ang ε_n ay may closed-form formula\n";
    cout << "  na maaaring i-encode nang walang decryption\n\n";

    return 0;
}
