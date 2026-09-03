// ============================================
// φ-MATH EMERGENT TEST
// Hanapin ang natural na conversion property
// sa pamamagitan ng mathematical testing
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-MATH EMERGENT TEST\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double PHI_INV = 1.0 / PHI;

    // ============================================
    // TEST 1: LOG → NORMAL VIA FIBONACCI
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: LOG → NORMAL VIA FIBONACCI\n";
    cout << "========================================\n\n";

    // Fibonacci sequence
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  log_φ(x) | x | F_n ≈ x? | F_{n+1}? | Match?\n";
    cout << "  ----------|---|-----------|-----------|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 55.0, 89.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)round(log_phi_x);
        long long f_n = fib[n];
        long long f_n1 = fib[n+1];
        
        bool match = (abs(f_n - x) < 0.1 || abs(f_n1 - x) < 0.1);
        
        cout << "  " << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(4) << x << " | "
             << setw(10) << f_n << " | "
             << setw(10) << f_n1 << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST 2: φ-PERIODICITY MAPPING
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 2: φ-PERIODICITY MAPPING\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | fmod(log_φ(x), φ) | φ^fmod | Match?\n";
    cout << "  --|-----------|---------------------|---------|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 35.0}) {
        double log_phi_x = log(x) / LN_PHI;
        double fmod_val = fmod(log_phi_x, PHI);
        double recovered = pow(PHI, fmod_val);
        bool match = abs(recovered - x) < 0.5;
        
        cout << "  " << setw(2) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(19) << fmod_val << " | "
             << setw(7) << recovered << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST 3: DUAL REPRESENTATION
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 3: DUAL REPRESENTATION\n";
    cout << "========================================\n\n";

    cout << "  x | a + bφ (a,b int) | a | b | Recovered?\n";
    cout << "  --|------------------|---|---|------------\n";

    for (double x : {3.0, 5.0, 7.0, 12.0, 35.0}) {
        // Hanapin ang a at b kung saan a + bφ ≈ x
        int best_a = 0, best_b = 0;
        double best_err = 1e9;
        
        for (int b = -10; b <= 10; b++) {
            for (int a = -10; a <= 10; a++) {
                double val = a + b * PHI;
                double err = abs(val - x);
                if (err < best_err) {
                    best_err = err;
                    best_a = a;
                    best_b = b;
                }
            }
        }
        
        double recovered = best_a + best_b * PHI;
        bool match = abs(recovered - x) < 0.01;
        
        cout << "  " << setw(2) << x << " | "
             << setw(16) << fixed << setprecision(4) << recovered << " | "
             << setw(2) << best_a << " | "
             << setw(2) << best_b << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST 4: φ² = φ + 1 EMERGENT
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 4: φ² = φ + 1 EMERGENT\n";
    cout << "========================================\n\n";

    cout << "  n | φ^n | Integer part | Fractional | φ^(n-1) + φ^(n-2) | Match?\n";
    cout << "  --|-----|--------------|------------|---------------------|--------\n";

    for (int n = 2; n <= 10; n++) {
        double phi_n = pow(PHI, n);
        double phi_n1 = pow(PHI, n-1);
        double phi_n2 = pow(PHI, n-2);
        double sum = phi_n1 + phi_n2;
        bool match = abs(phi_n - sum) < 0.001;
        
        cout << "  " << setw(2) << n << " | "
             << setw(6) << fixed << setprecision(4) << phi_n << " | "
             << setw(12) << (int)phi_n << " | "
             << setw(10) << (phi_n - (int)phi_n) << " | "
             << setw(19) << sum << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST 5: LOG → NORMAL VIA INTEGER + FRACTION
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST 5: LOG → NORMAL VIA INT + FRAC\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | int | frac | φ^int | φ^frac | Product | Match?\n";
    cout << "  --|-----------|-----|------|-------|--------|---------|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 35.0, 100.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int int_part = (int)floor(log_phi_x);
        double frac_part = log_phi_x - int_part;
        
        double phi_int = pow(PHI, int_part);
        double phi_frac = pow(PHI, frac_part);
        double product = phi_int * phi_frac;
        bool match = abs(product - x) < 0.01;
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(3) << int_part << " | "
             << setw(5) << frac_part << " | "
             << setw(5) << phi_int << " | "
             << setw(6) << phi_frac << " | "
             << setw(7) << product << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  Key observations:\n";
    cout << "  1. φ^n = F_n φ + F_{n-1} (exact)\n";
    cout << "  2. fmod(log_φ(x), φ) — hindi direct recovery\n";
    cout << "  3. a + bφ representation — limited to Fibonacci-like values\n";
    cout << "  4. φ² = φ + 1 — recurrence property\n";
    cout << "  5. int + frac decomposition — kailangan ng product\n\n";
    cout << "  Emergent property na hinahanap:\n";
    cout << "  - Natural na conversion mula log papuntang normal\n";
    cout << "  - Hindi kailangan ng exponentiation\n\n";

    return 0;
}
