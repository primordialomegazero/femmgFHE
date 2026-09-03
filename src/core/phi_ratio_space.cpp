// ============================================
// φ-RATIO SPACE
// I-encode ang ratio b/a sa self-ref space
// Hanapin ang natural na addition pattern
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
    cout << "  φ-RATIO SPACE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // RATIO ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  RATIO ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  x | n | a | b | b/a | F_n/F_{n-1}\n";
    cout << "  --|---|-----|-----|-----|---------\n";

    for (double x : {5.0, 7.0, 35.0, 3.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        double frac = log_phi_x - n;
        double phi_frac = pow(PHI, frac);
        
        double a = fib[n-1] * phi_frac;
        double b = fib[n] * phi_frac;
        double ratio = b / a;
        double fib_ratio = (double)fib[n] / fib[n-1];
        
        cout << "  " << setw(3) << x << " | "
             << setw(1) << n << " | "
             << setw(6) << fixed << setprecision(4) << a << " | "
             << setw(6) << b << " | "
             << setw(4) << ratio << " | "
             << setw(7) << fib_ratio << "\n";
    }

    // ============================================
    // ADDITION SA RATIO SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION SA RATIO SPACE\n";
    cout << "========================================\n\n";

    // 5: n=3, F_2=1, F_3=2
    double n5 = 3, frac5 = 3.3446 - 3;
    double phi_frac5 = pow(PHI, frac5);
    double a5 = fib[2] * phi_frac5;
    double b5 = fib[3] * phi_frac5;
    
    // 7: n=4, F_3=2, F_4=3
    double n7 = 4, frac7 = 4.0438 - 4;
    double phi_frac7 = pow(PHI, frac7);
    double a7 = fib[3] * phi_frac7;
    double b7 = fib[4] * phi_frac7;
    
    cout << "  5: a=" << a5 << ", b=" << b5 << "\n";
    cout << "  7: a=" << a7 << ", b=" << b7 << "\n\n";

    // Subok 1: Direct addition
    double a_direct = a5 + a7;
    double b_direct = b5 + b7;
    cout << "  Direct: a=" << a_direct << ", b=" << b_direct << "\n";
    cout << "  a + bφ = " << (a_direct + b_direct * PHI) << "\n\n";

    // Subok 2: φ-weighted addition
    double a_weighted = a5 + a7 * PHI;
    double b_weighted = b5 + b7 * PHI;
    cout << "  φ-weighted: a=" << a_weighted << ", b=" << b_weighted << "\n";
    cout << "  a + bφ = " << (a_weighted + b_weighted * PHI) << "\n\n";

    // Subok 3: n-weighted (index addition)
    int n_sum = (int)n5 + (int)n7;
    double frac_sum = frac5 + frac7;
    double phi_frac_sum = pow(PHI, frac_sum);
    double a_sum = fib[n_sum - 1] * phi_frac_sum;
    double b_sum = fib[n_sum] * phi_frac_sum;
    
    cout << "  Index addition: n=" << n_sum << ", frac=" << frac_sum << "\n";
    cout << "  a=" << a_sum << ", b=" << b_sum << "\n";
    cout << "  a + bφ = " << (a_sum + b_sum * PHI) << "\n\n";

    cout << "  Expected: 35\n";
    cout << "  Match: " << (abs(a_sum + b_sum * PHI - 35.0) < 0.01 ? "✅" : "❌") << "\n\n";

    return 0;
}
