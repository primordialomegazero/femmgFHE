// ============================================
// φ-FIBONACCI MODULO
// Ang Fibonacci modulo bilang natural na
// periodicity para sa log space
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
    cout << "  φ-FIBONACCI MODULO\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // TEST 1: FIBONACCI MODULO PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  FIBONACCI MODULO PATTERN\n";
    cout << "========================================\n\n";

    cout << "  F_n mod φ:\n";
    cout << "  n | F_n | F_n mod φ | φ^n mod φ\n";
    cout << "  --|-----|-----------|----------\n";

    for (int n = 0; n <= 20; n++) {
        double F_mod = fmod((double)fib[n], PHI);
        double phi_mod = fmod(pow(PHI, n), PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fib[n] << " | "
             << setw(9) << fixed << setprecision(4) << F_mod << " | "
             << setw(8) << phi_mod << "\n";
    }

    // ============================================
    // TEST 2: FIBONACCI MODULO SA LOG SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  FIBONACCI MODULO SA LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  log_φ(F_n) mod 1:\n";
    cout << "  n | log_φ(F_n) | frac | φ^frac\n";
    cout << "  --|-----------|------|-------\n";

    for (int n = 2; n <= 15; n++) {
        double log_F = log((double)fib[n]) / LN_PHI;
        double frac = log_F - floor(log_F);
        double phi_frac = pow(PHI, frac);
        
        cout << "  " << setw(2) << n << " | "
             << setw(9) << fixed << setprecision(4) << log_F << " | "
             << setw(5) << frac << " | "
             << setw(6) << phi_frac << "\n";
    }

    // ============================================
    // TEST 3: MODULO PERIODICITY
    // ============================================

    cout << "\n========================================\n";
    cout << "  MODULO PERIODICITY\n";
    cout << "========================================\n\n";

    cout << "  F_n mod φ ay may periodicity:\n";
    cout << "  n | F_n mod φ | F_{n+5} mod φ | Diff\n";
    cout << "  --|-----------|---------------|-----\n";

    for (int n = 0; n <= 10; n++) {
        double F_n = fmod((double)fib[n], PHI);
        double F_n5 = fmod((double)fib[n+5], PHI);
        double diff = F_n5 - F_n;
        
        cout << "  " << setw(2) << n << " | "
             << setw(9) << fixed << setprecision(4) << F_n << " | "
             << setw(13) << F_n5 << " | "
             << setw(5) << diff << "\n";
    }

    return 0;
}
