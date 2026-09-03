// ============================================
// φ-FIBONACCI ADDITION FORMULA
// Hanapin kung may paraan para ma-compute
// ang F_{n+m} mula sa F_n at F_m
// nang walang multiplication
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
    cout << "  φ-FIBONACCI ADDITION FORMULA\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    // Fibonacci sequence
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // TEST: F_{n+m} = F_n × F_{m+1} + F_{n-1} × F_m
    // ============================================

    cout << "========================================\n";
    cout << "  F_{n+m} FORMULA\n";
    cout << "========================================\n\n";

    cout << "  n | m | F_{n+m} | F_n×F_{m+1}+F_{n-1}×F_m | Match?\n";
    cout << "  --|---|---------|-------------------------|--------\n";

    for (int n = 1; n <= 5; n++) {
        for (int m = 1; m <= 5; m++) {
            long long fnm = fib[n+m];
            long long formula = fib[n] * fib[m+1] + fib[n-1] * fib[m];
            bool match = (fnm == formula);
            
            cout << "  " << setw(2) << n << " | "
                 << setw(1) << m << " | "
                 << setw(7) << fnm << " | "
                 << setw(21) << formula << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST: F_{n+m} sa φ-SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  F_{n+m} sa φ-SPACE\n";
    cout << "========================================\n\n";

    cout << "  n | m | φ^n | φ^m | φ^(n+m) | F_{n+m}\n";
    cout << "  --|---|-----|-----|---------|-------\n";

    for (int n = 3; n <= 5; n++) {
        for (int m = 4; m <= 6; m++) {
            double phi_n = pow(PHI, n);
            double phi_m = pow(PHI, m);
            double phi_nm = pow(PHI, n+m);
            
            cout << "  " << setw(2) << n << " | "
                 << setw(1) << m << " | "
                 << setw(5) << fixed << setprecision(2) << phi_n << " | "
                 << setw(5) << phi_m << " | "
                 << setw(7) << phi_nm << " | "
                 << setw(5) << fib[n+m] << "\n";
        }
    }

    // ============================================
    // TEST: EMERGENT PATTERN SA b COMPONENT
    // ============================================

    cout << "\n========================================\n";
    cout << "  EMERGENT PATTERN SA b COMPONENT\n";
    cout << "========================================\n\n";

    // Para sa (5 × 7):
    // b(5) = 2, b(7) = 3
    // b(35) = 13 = F_7
    // Ang 7 = 3 + 4 = n(5) + n(7) = 3 + 4
    // b(35) = F_7 = 13
    
    // Subok: b(35) = b(5) + b(7) + correction
    long long b5 = 2, b7 = 3, b35 = 13;
    long long correction = b35 - b5 - b7;
    
    cout << "  b(5) = " << b5 << "\n";
    cout << "  b(7) = " << b7 << "\n";
    cout << "  b(35) = " << b35 << "\n";
    cout << "  Correction: " << correction << " = F_5 + F_6 = 5 + 8\n\n";

    // Subok: b(35) = F_3 × F_5 + F_2 × F_4
    long long formula = fib[3] * fib[5] + fib[2] * fib[4];
    cout << "  F_3 × F_5 + F_2 × F_4 = " << formula << "\n";
    cout << "  Match: " << (formula == b35 ? "✅" : "❌") << "\n\n";

    return 0;
}
