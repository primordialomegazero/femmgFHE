// ============================================
// φ-ELEGANT ADD
// Hanapin ang mas eleganteng paraan
// para sa addition sa log space
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
    cout << "  φ-ELEGANT ADD\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // TEST 1: SYMMETRIC PROPERTY
    // ============================================

    cout << "========================================\n";
    cout << "  SYMMETRIC PROPERTY\n";
    cout << "========================================\n\n";

    cout << "  a + b = b + a (commutative)\n";
    cout << "  log_φ(a+b) = log_φ(b+a)\n\n";

    cout << "  log_φ(a+b) - log_φ(a) = correction(b/a)\n";
    cout << "  log_φ(a+b) - log_φ(b) = correction(a/b)\n\n";

    for (double a : {2.0, 3.0, 5.0}) {
        for (double b : {2.0, 3.0, 5.0}) {
            double corr_ab = log(a+b)/LN_PHI - log(a)/LN_PHI;
            double corr_ba = log(a+b)/LN_PHI - log(b)/LN_PHI;
            
            cout << "  a=" << a << ", b=" << b 
                 << " | corr(b/a)=" << fixed << setprecision(4) << corr_ab
                 << " | corr(a/b)=" << corr_ba << "\n";
        }
    }

    // ============================================
    // TEST 2: ANTISYMMETRY
    // ============================================

    cout << "\n========================================\n";
    cout << "  ANTISYMMETRY\n";
    cout << "========================================\n\n";

    cout << "  corr(b/a) + corr(a/b) = ?\n";
    cout << "  a | b | corr(b/a) | corr(a/b) | Sum\n";
    cout << "  --|---|-----------|-----------|-----\n";

    for (double a : {2.0, 3.0, 5.0, 8.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0}) {
            double corr_ab = log(a+b)/LN_PHI - log(a)/LN_PHI;
            double corr_ba = log(a+b)/LN_PHI - log(b)/LN_PHI;
            double sum = corr_ab + corr_ba;
            
            cout << "  " << a << " | " << b 
                 << " | " << fixed << setprecision(4) << corr_ab
                 << " | " << corr_ba
                 << " | " << sum << "\n";
        }
    }

    // ============================================
    // TEST 3: PHI-EXPANSION NG CORRECTION
    // ============================================

    cout << "\n========================================\n";
    cout << "  PHI-EXPANSION NG CORRECTION\n";
    cout << "========================================\n\n";

    cout << "  corr(b/a) = log_φ(1 + b/a)\n";
    cout << "  = log_φ((a+b)/a)\n";
    cout << "  = log_φ(a+b) - log_φ(a)\n\n";

    // Subok: i-express ang correction bilang φ-power
    cout << "  1 + b/a = φ^k?\n";
    cout << "  a | b | 1+b/a | log_φ(1+b/a) | nearest k\n";
    cout << "  --|---|-------|-------------|----------\n";

    for (double a : {2.0, 3.0, 5.0, 8.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0}) {
            double ratio = 1 + b/a;
            double log_val = log(ratio) / LN_PHI;
            double nearest_k = round(log_val);
            
            cout << "  " << a << " | " << b 
                 << " | " << fixed << setprecision(4) << ratio
                 << " | " << log_val
                 << " | " << nearest_k << "\n";
        }
    }

    // ============================================
    // TEST 4: LUCAS-FIBONACCI CORRECTION
    // ============================================

    cout << "\n========================================\n";
    cout << "  LUCAS-FIBONACCI CORRECTION\n";
    cout << "========================================\n\n";

    cout << "  a+b = F_{m+n}? o L_{m+n}?\n";
    cout << "  a | b | a+b | nearest F | nearest L\n";
    cout << "  --|---|-----|-----------|----------\n";

    vector<long long> fib = {0, 1};
    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    for (double a : {2.0, 3.0, 5.0, 8.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0}) {
            double sum = a + b;
            
            // Find nearest Fibonacci
            long long nearest_f = 0;
            double min_f = 1e9;
            for (long long f : fib) {
                double d = abs(f - sum);
                if (d < min_f) { min_f = d; nearest_f = f; }
            }
            
            // Find nearest Lucas
            long long nearest_l = 0;
            double min_l = 1e9;
            for (long long l : lucas) {
                double d = abs(l - sum);
                if (d < min_l) { min_l = d; nearest_l = l; }
            }
            
            cout << "  " << a << " | " << b 
                 << " | " << sum
                 << " | F=" << nearest_f << " (err=" << min_f << ")"
                 << " | L=" << nearest_l << " (err=" << min_l << ")\n";
        }
    }

    return 0;
}
