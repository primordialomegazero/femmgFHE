// ============================================
// φ-SELF-REFERENTIAL SPACE
// I-encode ang value sa φ-space:
// v = a + bφ kung saan φ² = φ + 1
// Hanapin ang natural na addition at multiplication
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-SELF-REFERENTIAL SPACE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // SELF-REFERENTIAL DECOMPOSITION
    // ============================================

    cout << "========================================\n";
    cout << "  SELF-REFERENTIAL DECOMPOSITION\n";
    cout << "========================================\n\n";

    cout << "  x | a | b | a + bφ | φ(a) + φ(b)\n";
    cout << "  --|---|---|---------|------------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 35.0, 55.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        double frac = log_phi_x - n;
        
        // Decompose: x = φ^n × φ^frac
        // = (F_{n-1} + F_n φ) × φ^frac
        // = a + bφ kung saan a at b ay nakadepende sa frac
        
        double phi_n = pow(PHI, n);
        double phi_frac = pow(PHI, frac);
        
        // x = phi_n × phi_frac
        // phi_n = F_{n-1} + F_n φ
        // x = (F_{n-1} + F_n φ) × phi_frac
        // = F_{n-1}×phi_frac + F_n×phi_frac×φ
        
        vector<long long> fib = {0, 1};
        for (int i = 2; i <= 20; i++) {
            fib.push_back(fib[i-1] + fib[i-2]);
        }
        
        double a = fib[n-1] * phi_frac;
        double b = fib[n] * phi_frac;
        
        cout << "  " << setw(3) << x << " | "
             << setw(5) << fixed << setprecision(4) << a << " | "
             << setw(5) << b << " | "
             << setw(7) << (a + b * PHI) << " | "
             << setw(10) << (PHI * a + PHI * b) << "\n";
    }

    // ============================================
    // TEST: (5 × 7) + 3 SA SELF-REF SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  (5 × 7) + 3 SA SELF-REF SPACE\n";
    cout << "========================================\n\n";

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // 5: log_φ(5) = 3.3446, n=3, frac=0.3446
    double frac_5 = 3.3446 - 3;
    double phi_frac_5 = pow(PHI, frac_5);
    double a_5 = fib[2] * phi_frac_5;  // F_2 = 1
    double b_5 = fib[3] * phi_frac_5;  // F_3 = 2
    
    // 7: log_φ(7) = 4.0438, n=4, frac=0.0438
    double frac_7 = 4.0438 - 4;
    double phi_frac_7 = pow(PHI, frac_7);
    double a_7 = fib[3] * phi_frac_7;  // F_3 = 2
    double b_7 = fib[4] * phi_frac_7;  // F_4 = 3
    
    cout << "  5: a=" << a_5 << ", b=" << b_5 << "\n";
    cout << "  7: a=" << a_7 << ", b=" << b_7 << "\n\n";

    // Addition sa self-ref space
    double a_sum = a_5 + a_7;
    double b_sum = b_5 + b_7;
    
    cout << "  After 5 × 7 (addition):\n";
    cout << "  a = " << a_sum << "\n";
    cout << "  b = " << b_sum << "\n";
    cout << "  a + bφ = " << (a_sum + b_sum * PHI) << "\n\n";

    // Para sa 35:
    double frac_35 = 7.3883 - 7;
    double phi_frac_35 = pow(PHI, frac_35);
    double a_35 = fib[6] * phi_frac_35;  // F_6 = 8
    double b_35 = fib[7] * phi_frac_35;  // F_7 = 13
    
    cout << "  35: a=" << a_35 << ", b=" << b_35 << "\n";
    cout << "  a + bφ = " << (a_35 + b_35 * PHI) << "\n\n";

    return 0;
}
