// PERIOD-K GENERALIZATION + COMPLEX + MULTI-DIMENSIONAL
// Hanapin ang lahat ng bounded recurrences

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  PERIOD-K GENERALIZATION\n";
    cout << "  Complex + Multi-Dimensional\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;

    // ============================================
    // 1. PERIOD-K SEARCH: x_n - a·x_{n-1} + K
    // ============================================
    cout << "1. PERIOD-K SEARCH:\n";
    cout << "===================\n\n";

    cout << "  Testing x_{n+1} = a·x_n - x_{n-1} + K\n";
    cout << "  Para sa iba't ibang 'a' values:\n\n";

    for (double a : {-2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0}) {
        double x_prev = 0.0;
        double x_curr = phi;
        bool bounded = true;
        double max_val = 0;
        
        for (int i = 0; i < 100; i++) {
            double x_next = a * x_curr - x_prev + phi;
            if (std::abs(x_next) > 50) { bounded = false; break; }
            if (std::abs(x_next) > max_val) max_val = std::abs(x_next);
            x_prev = x_curr;
            x_curr = x_next;
        }
        
        cout << "  a=" << a << ": " << (bounded ? "BOUNDED" : "UNBOUNDED");
        if (bounded) cout << " (max=" << max_val << ")";
        cout << "\n";
    }

    cout << "\n";

    // ============================================
    // 2. COMPLEX REPRESENTATION
    // ============================================
    cout << "2. COMPLEX REPRESENTATION:\n";
    cout << "=========================\n\n";

    complex<double> r1(0.5, 0.866);   // e^{iπ/3}
    complex<double> r2(0.5, -0.866);  // e^{-iπ/3}

    cout << "  r₁ = " << r1 << " (magnitude " << abs(r1) << ")\n";
    cout << "  r₂ = " << r2 << " (magnitude " << abs(r2) << ")\n\n";

    // I-verify ang period-6 sa complex domain
    cout << "  r₁^n para sa n=0..6:\n";
    for (int n = 0; n <= 6; n++) {
        complex<double> rn = pow(r1, n);
        cout << "    r₁^" << n << " = " << rn << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. FHE COMPLEX ENCODING
    // ============================================
    cout << "3. FHE COMPLEX ENCODING:\n";
    cout << "========================\n\n";

    cout << "  Ang CKKS ay naturally complex!\n";
    cout << "  Hindi natin kailangan ng separate encoding.\n";
    cout << "  Ang φ-domain ay may natural complex structure:\n";
    cout << "    φ = 2cos(π/5) = 2cos(36°)\n";
    cout << "    e^{iπ/3} = cos(60°) + i·sin(60°)\n\n";

    cout << "  Kaya ang period-6 recurrence ay may\n";
    cout << "  natural complex representation sa CKKS!\n\n";

    // ============================================
    // 4. MULTI-DIMENSIONAL
    // ============================================
    cout << "4. MULTI-DIMENSIONAL:\n";
    cout << "=====================\n\n";

    // 2D: dalawang recurrences na magkaugnay
    cout << "  2D SYSTEM:\n";
    cout << "  x_{n+1} = x_n - y_n + φ\n";
    cout << "  y_{n+1} = y_n - x_n + φ\n\n";

    double x = 0.0, y = phi;
    cout << "  Sequence: ";
    bool bounded_2d = true;
    for (int i = 0; i < 20; i++) {
        double x_next = x - y + phi;
        double y_next = y - x + phi;
        x = x_next;
        y = y_next;
        
        double mag = sqrt(x*x + y*y);
        if (mag > 20) { bounded_2d = false; break; }
        
        if (i < 10) {
            cout << "(" << x << "," << y << ") ";
        }
    }
    cout << "\n";
    cout << "  Bounded: " << (bounded_2d ? "YES" : "NO") << "\n\n";

    // ============================================
    // 5. BEST BOUNDED RECURRENCE SUMMARY
    // ============================================
    cout << "5. BEST BOUNDED RECURRENCES:\n";
    cout << "============================\n\n";

    cout << "  Ang mga bounded recurrences ay:\n";
    cout << "  1. x_{n+1} = x_n - x_{n-1} + K (period-6)\n";
    cout << "     - Bounded para sa LAHAT ng K\n";
    cout << "     - States: 2K, 2K, K, 0, 0, K\n\n";

    cout << "  Ang period-6 ay unibersal para sa form na ito!\n";
    cout << "  Hindi arbitrary — may mathematical foundation.\n\n";

    cout << "========================================\n";
    cout << "  KEY FORMULA:\n";
    cout << "  x_{n+1} = x_n - x_{n-1} + K\n";
    cout << "  May period-6 para sa LAHAT ng K\n";
    cout << "  Bounded, 0-level, at may complex roots\n";
    cout << "========================================\n";

    return 0;
}
