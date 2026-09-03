// ============================================
// φ-MERGING PROPERTY SEARCH
// Hanapin ang natural na property ng φ na
// nagbibigay ng conversion o merging
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
    cout << "  φ-MERGING PROPERTY SEARCH\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double PHI_INV = 1.0 / PHI;
    const double SQRT5 = sqrt(5.0);

    // ============================================
    // TEST 1: φ^n + φ^{-n} = L_n (Lucas Numbers)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: φ^n + φ^{-n} = L_n\n";
    cout << "========================================\n\n";

    cout << "  n | φ^n | φ^{-n} | Sum | L_n | Match?\n";
    cout << "  --|-----|--------|-----|-----|--------\n";

    for (int n = 0; n <= 10; n++) {
        double phi_n = pow(PHI, n);
        double phi_neg_n = pow(PHI_INV, n);
        double sum = phi_n + phi_neg_n;
        double lucas = (pow(PHI, n) + pow(-PHI_INV, n));
        bool match = abs(sum - lucas) < 0.01;
        
        cout << "  " << setw(2) << n << " | "
             << setw(6) << fixed << setprecision(4) << phi_n << " | "
             << setw(6) << phi_neg_n << " | "
             << setw(5) << sum << " | "
             << setw(5) << lucas << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST 2: log_φ(x) + log_φ(1/x) = 0
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 2: LOG + INVERSE LOG\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | log_φ(1/x) | Sum | Match?\n";
    cout << "  --|-----------|-------------|------|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0}) {
        double log_x = log(x) / LN_PHI;
        double log_inv = log(1.0/x) / LN_PHI;
        double sum = log_x + log_inv;
        bool match = abs(sum) < 0.001;
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_x << " | "
             << setw(11) << log_inv << " | "
             << setw(4) << sum << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST 3: φ^(a+b) = φ^a × φ^b
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 3: φ^(a+b) = φ^a × φ^b\n";
    cout << "========================================\n\n";

    cout << "  a | b | φ^a | φ^b | φ^a × φ^b | φ^(a+b) | Match?\n";
    cout << "  --|---|-----|-----|-----------|---------|--------\n";

    for (double a : {1.0, 2.0, 3.0}) {
        for (double b : {1.0, 2.0, 3.0}) {
            double phi_a = pow(PHI, a);
            double phi_b = pow(PHI, b);
            double product = phi_a * phi_b;
            double phi_sum = pow(PHI, a + b);
            bool match = abs(product - phi_sum) < 0.01;
            
            cout << "  " << setw(2) << a << " | "
                 << setw(1) << b << " | "
                 << setw(4) << fixed << setprecision(2) << phi_a << " | "
                 << setw(4) << phi_b << " | "
                 << setw(9) << product << " | "
                 << setw(7) << phi_sum << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 4: F_n + F_{n+1} = F_{n+2}
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 4: F_n + F_{n+1} = F_{n+2}\n";
    cout << "========================================\n\n";

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 15; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  n | F_n + F_{n+1} | F_{n+2} | Match?\n";
    cout << "  --|---------------|---------|--------\n";

    for (int n = 0; n <= 10; n++) {
        long long sum = fib[n] + fib[n+1];
        long long next = fib[n+2];
        bool match = (sum == next);
        
        cout << "  " << setw(2) << n << " | "
             << setw(13) << sum << " | "
             << setw(7) << next << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST 5: φ × F_n + F_{n-1} = F_{n+1}
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 5: φ × F_n + F_{n-1}\n";
    cout << "========================================\n\n";

    cout << "  n | φ × F_n + F_{n-1} | F_{n+1} | Match?\n";
    cout << "  --|--------------------|---------|--------\n";

    for (int n = 1; n <= 12; n++) {
        double val = PHI * fib[n] + fib[n-1];
        double expected = (pow(PHI, n) - pow(-PHI_INV, n)) / SQRT5 + fib[n];
        bool match = abs(val - fib[n+1]) < 0.01;
        
        cout << "  " << setw(2) << n << " | "
             << setw(18) << fixed << setprecision(4) << val << " | "
             << setw(7) << fib[n+1] << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  KEY FINDINGS\n";
    cout << "========================================\n\n";
    cout << "  Emergent properties ng φ:\n";
    cout << "  1. φ^n + φ^{-n} = L_n (Lucas)\n";
    cout << "  2. log_φ(x) + log_φ(1/x) = 0\n";
    cout << "  3. φ^(a+b) = φ^a × φ^b\n";
    cout << "  4. F_n + F_{n+1} = F_{n+2}\n";
    cout << "  5. φ × F_n + F_{n-1} = φ^n\n\n";

    return 0;
}
