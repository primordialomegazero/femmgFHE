// ============================================
// φ-FIBONACCI + CONSTANT CONVERSION
// Subok kung ang x = φ^n × φ^frac ay kayang
// i-compute gamit ang Fibonacci at constant
// nang walang EvalMult
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
    cout << "  φ-FIBONACCI + CONSTANT CONVERSION\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double SQRT5 = sqrt(5.0);
    const double INV_SQRT5 = 1.0 / SQRT5;
    const double PHI_FRAC_CONST = PHI - INV_SQRT5;

    // Fibonacci sequence
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // TEST: CONVERSION VIA FIB + CONST
    // ============================================

    cout << "========================================\n";
    cout << "  CONVERSION TEST\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | n | F_n | F_{n-1} | Recovered | Err\n";
    cout << "  --|-----------|----|-----|---------|-----------|-----\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0, 89.0, 144.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        
        // Recovered = F_n × PHI_FRAC_CONST + F_{n-1} × PHI_FRAC_CONST / φ
        double recovered = fib[n] * PHI_FRAC_CONST + fib[n-1] * (PHI_FRAC_CONST / PHI);
        
        double err = abs(recovered - x);
        
        cout << "  " << setw(4) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(2) << n << " | "
             << setw(4) << fib[n] << " | "
             << setw(7) << fib[n-1] << " | "
             << setw(9) << fixed << setprecision(4) << recovered << " | "
             << setw(5) << err << "\n";
    }

    // ============================================
    // TEST: EXACT FORMULA
    // ============================================

    cout << "\n========================================\n";
    cout << "  EXACT FORMULA\n";
    cout << "========================================\n\n";

    cout << "  Para sa x = φ^n × φ^frac:\n";
    cout << "  φ^frac ≈ " << PHI_FRAC_CONST << "\n";
    cout << "  φ^n = F_n φ + F_{n-1}\n";
    cout << "  x ≈ (F_n φ + F_{n-1}) × " << PHI_FRAC_CONST << "\n\n";

    cout << "  Kailangan ng multiplication sa φ at const.\n";
    cout << "  Sa encrypted domain, ito ay EvalMult.\n\n";

    cout << "  PERO kung ang constants ay pre-computed,\n";
    cout << "  at naka-encode sa slots,\n";
    cout << "  baka kaya nating i-add na lang.\n\n";

    return 0;
}
