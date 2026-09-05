// ============================================
// φ-SELF-REF CORRECTION
// Ang correction bilang φ-power series
// Self-referential consistency
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double C = 1.0 / LN_PHI;

    cout << "=== φ-SELF-REF CORRECTION ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Taylor series ng corr(k)
    // ============================================
    cout << "--- 1. Taylor series ---\n\n";
    cout << "  corr(k) = C × φ^(-k) - C/2 × φ^(-2k) + C/3 × φ^(-3k) - ...\n\n";
    
    auto taylor_corr = [&](int k, int terms) {
        double sum = 0;
        for (int j = 1; j <= terms; j++) {
            double sign = (j % 2 == 1) ? 1.0 : -1.0;
            sum += sign * C / j * pow(PHI, -j * k);
        }
        return sum;
    };
    
    auto exact_corr = [&](int k) {
        return log(1.0 + pow(PHI, -k)) / LN_PHI;
    };
    
    cout << "  k | exact | Taylor(3) | Taylor(5) | Taylor(10)\n";
    cout << "  --|-------|-----------|-----------|----------\n";
    
    for (int k = 1; k <= 10; k++) {
        cout << "  " << setw(2) << k << " | "
             << setw(8) << exact_corr(k) << " | "
             << setw(9) << taylor_corr(k, 3) << " | "
             << setw(9) << taylor_corr(k, 5) << " | "
             << setw(9) << taylor_corr(k, 10) << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Self-referential: corr(k) = f(corr(k+1))
    // ============================================
    cout << "--- 2. Self-referential na relasyon ---\n\n";
    cout << "  corr(k+1) / corr(k) → φ⁻¹\n\n";
    
    cout << "  k | corr(k+1)/corr(k) | φ⁻¹\n";
    cout << "  --|-------------------|-------\n";
    
    for (int k = 1; k <= 15; k++) {
        double ratio = exact_corr(k+1) / exact_corr(k);
        cout << "  " << setw(2) << k << " | "
             << setw(18) << ratio << " | "
             << setw(10) << 1.0/PHI << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang correction bilang fixed point
    // ============================================
    cout << "--- 3. Fixed point ng correction ---\n\n";
    cout << "  corr(k) - φ^(-k) = ?\n";
    cout << "  Kung corr(k) = φ^(-k), anong relasyon?\n\n";
    
    // corr(k) = φ^(-k) × (1 + something)
    cout << "  k | corr(k)/φ^(-k) | 1 + ?\n";
    cout << "  --|----------------|-------\n";
    
    for (int k = 1; k <= 15; k++) {
        double ratio = exact_corr(k) / pow(PHI, -k);
        cout << "  " << setw(2) << k << " | "
             << setw(14) << ratio << " | "
             << setw(10) << ratio - 1.0 << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang consistent na approximation
    // ============================================
    cout << "--- 4. Consistent approximation ---\n\n";
    cout << "  corr(k) ≈ φ^(-k) / (1 - φ^(-k)/2)\n\n";
    
    auto approx_corr = [&](int k) {
        double phi_neg_k = pow(PHI, -k);
        return phi_neg_k / (1.0 - phi_neg_k / 2.0);
    };
    
    cout << "  k | exact | approx | diff\n";
    cout << "  --|-------|--------|------\n";
    
    for (int k = 1; k <= 10; k++) {
        double e = exact_corr(k);
        double a = approx_corr(k);
        cout << "  " << setw(2) << k << " | "
             << setw(8) << e << " | "
             << setw(8) << a << " | "
             << setw(8) << (e - a) << "\n";
    }
    cout << "\n";

    return 0;
}
