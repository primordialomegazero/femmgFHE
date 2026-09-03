// ============================================
// φ-ELEGANT FORMULA
// Addition sa log space via Lucas-Fibonacci
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
    cout << "  φ-ELEGANT FORMULA\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double SQRT5 = sqrt(5.0);

    vector<long long> fib = {0, 1};
    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    cout << "  a | b | a+b | log_φ(a+b) | F-index | L-index | Match?\n";
    cout << "  --|---|-----|-----------|---------|---------|-------\n";

    for (double a : {2.0, 3.0, 5.0, 8.0, 13.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0, 13.0}) {
            double sum = a + b;
            double log_sum = log(sum) / LN_PHI;
            
            // Hanapin ang nearest Fibonacci index
            int best_f = 0;
            double min_f = 1e9;
            for (int i = 0; i < fib.size(); i++) {
                double d = abs(fib[i] - sum);
                if (d < min_f) { min_f = d; best_f = i; }
            }
            
            // Hanapin ang nearest Lucas index
            int best_l = 0;
            double min_l = 1e9;
            for (int i = 0; i < lucas.size(); i++) {
                double d = abs(lucas[i] - sum);
                if (d < min_l) { min_l = d; best_l = i; }
            }
            
            bool match = (min_f < 0.01 || min_l < 0.01);
            
            cout << "  " << setw(2) << a << " | " << setw(2) << b 
                 << " | " << setw(3) << sum
                 << " | " << fixed << setprecision(4) << log_sum
                 << " | F_" << best_f << " (err=" << min_f << ")"
                 << " | L_" << best_l << " (err=" << min_l << ")"
                 << " | " << (match ? "✅" : "") << "\n";
        }
    }

    // ============================================
    // FORMULA TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  FORMULA TEST\n";
    cout << "========================================\n\n";

    cout << "  Para sa Lucas numbers:\n";
    cout << "  L_n ≈ φ^n → log_φ(L_n) ≈ n\n\n";

    cout << "  n | L_n | log_φ(L_n) | n | Diff\n";
    cout << "  --|-----|-----------|----|------\n";

    for (int n = 0; n <= 10; n++) {
        double log_L = log(lucas[n]) / LN_PHI;
        double diff = log_L - n;
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << lucas[n] << " | "
             << fixed << setprecision(4) << log_L << " | "
             << setw(2) << n << " | "
             << setw(5) << diff << "\n";
    }

    cout << "\n  Para sa Fibonacci numbers:\n";
    cout << "  F_n ≈ φ^n/√5 → log_φ(F_n) ≈ n - log_φ(√5)\n\n";

    cout << "  n | F_n | log_φ(F_n) | n - log_φ(√5) | Diff\n";
    cout << "  --|-----|-----------|---------------|------\n";

    double log_sqrt5 = log(SQRT5) / LN_PHI;

    for (int n = 0; n <= 10; n++) {
        double log_F = log(fib[n]) / LN_PHI;
        double approx = n - log_sqrt5;
        double diff = log_F - approx;
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fib[n] << " | "
             << fixed << setprecision(4) << log_F << " | "
             << fixed << setprecision(4) << approx << " | "
             << setw(5) << diff << "\n";
    }

    return 0;
}
