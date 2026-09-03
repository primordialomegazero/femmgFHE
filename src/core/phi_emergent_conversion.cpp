// ============================================
// φ-EMERGENT CONVERSION
// I-convert ang log_φ(x) papuntang x gamit
// ang Fibonacci recurrence at φ-identity
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
    cout << "  φ-EMERGENT CONVERSION\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double SQRT5 = sqrt(5.0);
    const double INV_SQRT5 = 1.0 / SQRT5;

    // Fibonacci sequence
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // TEST: EXACT CONVERSION VIA φ^n = F_n φ + F_{n-1}
    // ============================================

    cout << "========================================\n";
    cout << "  CONVERSION VIA φ^n = F_n φ + F_{n-1}\n";
    cout << "========================================\n\n";

    cout << "  log_φ(x) | n | frac | φ^n (fib) | φ^frac | Recovered | x | Match?\n";
    cout << "  ----------|----|------|-----------|--------|-----------|----|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        double frac = log_phi_x - n;
        
        // φ^n gamit ang Fibonacci
        double phi_n = fib[n] * PHI + fib[n-1];
        
        // φ^frac — exact
        double phi_frac = pow(PHI, frac);
        
        // Recovered = φ^n × φ^frac
        double recovered = phi_n * phi_frac;
        double err = abs(recovered - x);
        bool match = err < 0.01;
        
        cout << "  " << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(2) << n << " | "
             << setw(4) << frac << " | "
             << setw(9) << phi_n << " | "
             << setw(6) << phi_frac << " | "
             << setw(9) << recovered << " | "
             << setw(4) << x << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST: ADDITION ONLY VERSION
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION ONLY VERSION\n";
    cout << "========================================\n\n";

    cout << "  x | φ^n (add) | φ^frac (const) | Sum | x | Match?\n";
    cout << "  --|-----------|----------------|-----|----|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        double frac = log_phi_x - n;
        
        // φ^n gamit ang Fibonacci recurrence (addition lang)
        double phi_n = fib[n] * PHI + fib[n-1];
        
        // φ^frac approximation: φ - 1/√5 (para sa malalaking x)
        double phi_frac_approx = PHI - INV_SQRT5;
        
        // Recovered = φ^n × φ^frac ≈ φ^n × (φ - 1/√5)
        double recovered = phi_n * phi_frac_approx;
        double err = abs(recovered - x);
        bool match = err < 0.01;
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << phi_n << " | "
             << setw(14) << phi_frac_approx << " | "
             << setw(4) << (phi_n + phi_frac_approx) << " | "
             << setw(4) << x << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n\n";
    cout << "  Ang φ^n ay kayang i-compute via addition\n";
    cout << "  (Fibonacci recurrence + φ constant).\n\n";
    cout << "  Ang φ^frac ay bounded sa [1, φ).\n";
    cout << "  Para sa malalaking x, φ^frac ≈ φ - 1/√5.\n\n";
    cout << "  Ang recovery ay: x = (F_n φ + F_{n-1}) × φ^frac\n";
    cout << "  Ito ay nangangailangan ng multiplication.\n";
    cout << "  PERO kung ang φ^frac ay naka-encode sa slots,\n";
    cout << "  baka kaya nating i-add na lang.\n\n";

    return 0;
}
