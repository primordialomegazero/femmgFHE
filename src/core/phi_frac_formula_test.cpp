// ============================================
// φ-FRACTIONAL FORMULA TEST
// I-verify kung ang φ^frac ay may simpleng
// formula na hindi nangangailangan ng exponentiation
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
    cout << "  φ-FRACTIONAL FORMULA TEST\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double SQRT5 = sqrt(5.0);
    const double INV_SQRT5 = 1.0 / SQRT5;

    // ============================================
    // TEST: φ^frac ≈ φ - 1/√5 ?
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: φ^frac ≈ φ - 1/√5\n";
    cout << "========================================\n\n";

    cout << "  φ - 1/√5 = " << (PHI - INV_SQRT5) << "\n";
    cout << "  φ^0.3277 = " << pow(PHI, 0.3277) << "\n\n";

    double diff = abs((PHI - INV_SQRT5) - pow(PHI, 0.3277));
    cout << "  Difference: " << diff << "\n\n";

    // ============================================
    // TEST: GENERAL FORMULA
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: GENERAL FORMULA\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | int | frac | φ^frac | φ - 1/√5 | Err\n";
    cout << "  --|-----------|-----|------|---------|-----------|-----\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0, 89.0, 144.0, 233.0, 377.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int int_part = (int)floor(log_phi_x);
        double frac = log_phi_x - int_part;
        double phi_frac = pow(PHI, frac);
        double approx = PHI - INV_SQRT5;
        double err = abs(phi_frac - approx);
        
        cout << "  " << setw(4) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(3) << int_part << " | "
             << setw(5) << frac << " | "
             << setw(7) << phi_frac << " | "
             << setw(9) << approx << " | "
             << setw(5) << err << "\n";
    }

    // ============================================
    // TEST: EXACT FORMULA PARA SA FIBONACCI
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 3: FIBONACCI EXACT\n";
    cout << "========================================\n\n";

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  n | F_n | φ^n/√5 | F_n - φ^n/√5 | Err\n";
    cout << "  --|-----|---------|---------------|-----\n";

    for (int n = 1; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double approx = phi_n / SQRT5;
        double err = abs(fib[n] - approx);
        
        cout << "  " << setw(2) << n << " | "
             << setw(4) << fib[n] << " | "
             << setw(10) << fixed << setprecision(4) << approx << " | "
             << setw(12) << (fib[n] - approx) << " | "
             << setw(5) << err << "\n";
    }

    cout << "\n========================================\n";
    cout << "  KEY FINDING\n";
    cout << "========================================\n\n";
    cout << "  Para sa Fibonacci numbers:\n";
    cout << "  F_n = φ^n / √5 (approximately)\n";
    cout << "  Ang error ay papunta sa 0 habang lumalaki ang n.\n\n";
    cout << "  Para sa fractional part:\n";
    cout << "  φ^frac ≈ φ - 1/√5 ≈ 1.1708\n";
    cout << "  Ito ay convergence sa φ - 1/√5.\n\n";

    return 0;
}
