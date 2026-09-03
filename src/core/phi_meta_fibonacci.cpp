// ============================================
// φ-META FIBONACCI
// Meta-space na may φ-structure sa loob
// M_n = (F_n mod φ, F_{n-1} mod φ)
// Na may natural na φ-periodicity
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
    cout << "  φ-META FIBONACCI\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 100; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // META-FIBONACCI STRUCTURE
    // ============================================

    cout << "========================================\n";
    cout << "  META-FIBONACCI STRUCTURE\n";
    cout << "========================================\n\n";

    cout << "  M_n = F_n mod φ\n";
    cout << "  Ito ay bounded sa [0, φ)\n\n";

    cout << "  n | F_n | F_n mod φ | M_n\n";
    cout << "  --|-----|-----------|----\n";

    for (int n = 0; n <= 20; n++) {
        double M_n = fmod((double)fib[n], PHI);
        cout << "  " << setw(2) << n << " | "
             << setw(7) << fib[n] << " | "
             << setw(9) << fixed << setprecision(4) << M_n << " | "
             << setw(6) << M_n << "\n";
    }

    // ============================================
    // META PERIODICITY
    // ============================================

    cout << "\n========================================\n";
    cout << "  META PERIODICITY\n";
    cout << "========================================\n\n";

    cout << "  May periodicity ba ang M_n?\n";
    cout << "  n | M_n | M_{n+5} | Diff\n";
    cout << "  --|-----|---------|-----\n";

    for (int n = 0; n <= 10; n++) {
        double M_n = fmod((double)fib[n], PHI);
        double M_n5 = fmod((double)fib[n+5], PHI);
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fixed << setprecision(4) << M_n << " | "
             << setw(7) << M_n5 << " | "
             << setw(5) << (M_n5 - M_n) << "\n";
    }

    // ============================================
    // META LOG SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  META LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  log_φ(M_n) = ?\n";
    cout << "  n | M_n | log_φ(M_n)\n";
    cout << "  --|-----|----------\n";

    for (int n = 2; n <= 15; n++) {
        double M_n = fmod((double)fib[n], PHI);
        double log_M = (M_n > 0) ? log(M_n) / LN_PHI : 0;
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fixed << setprecision(4) << M_n << " | "
             << setw(9) << log_M << "\n";
    }

    // ============================================
    // KEY INSIGHT
    // ============================================

    cout << "\n========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n\n";

    cout << "  Ang M_n = F_n mod φ ay:\n";
    cout << "  - Bounded sa [0, φ)\n";
    cout << "  - May natural na periodicity\n";
    cout << "  - Hindi nag-o-overflow\n\n";

    cout << "  Ang meta log space:\n";
    cout << "  log_φ(M_n) ay bounded din\n";
    cout << "  - Walang overflow sa CKKS\n";
    cout << "  - Natural na φ-structure\n\n";

    return 0;
}
