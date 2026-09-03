// ============================================
// φ-ENCODING EXPLORE
// Subukan ang iba't ibang encoding spaces
// para sa mixed operations
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
    cout << "  φ-ENCODING EXPLORE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double SQRT5 = sqrt(5.0);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // Lucas numbers
    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 30; i++) {
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    // ============================================
    // ENCODING 1: PHI-POWER SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  ENCODING 1: PHI-POWER SPACE\n";
    cout << "========================================\n\n";

    cout << "  x | φ^n | φ^frac | φ^n × φ^frac | x | Match?\n";
    cout << "  --|-----|--------|--------------|---|--------\n";

    for (double x : {5.0, 7.0, 35.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        double frac = log_phi_x - n;
        double phi_n = pow(PHI, n);
        double phi_frac = pow(PHI, frac);
        double recovered = phi_n * phi_frac;
        bool match = abs(recovered - x) < 0.01;
        
        cout << "  " << setw(3) << x << " | "
             << setw(6) << fixed << setprecision(4) << phi_n << " | "
             << setw(6) << phi_frac << " | "
             << setw(12) << recovered << " | "
             << setw(3) << x << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // ENCODING 2: LUCAS SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ENCODING 2: LUCAS SPACE\n";
    cout << "========================================\n\n";

    cout << "  n | L_n | F_n | L_n² - 4F_n² | φ^n\n";
    cout << "  --|-----|-----|-------------|-----\n";

    for (int n = 0; n <= 10; n++) {
        double L_n = pow(PHI, n) + pow(-1.0/PHI, n);
        long long L_exact = lucas[n];
        long long F_exact = fib[n];
        double L2_minus_4F2 = L_exact * L_exact - 4 * F_exact * F_exact;
        
        cout << "  " << setw(2) << n << " | "
             << setw(4) << L_exact << " | "
             << setw(4) << F_exact << " | "
             << setw(11) << L2_minus_4F2 << " | "
             << setw(6) << fixed << setprecision(2) << pow(PHI, n) << "\n";
    }

    // ============================================
    // ENCODING 3: FIBONACCI PRODUCT SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ENCODING 3: FIBONACCI PRODUCT\n";
    cout << "========================================\n\n";

    cout << "  n | m | F_n × F_m | F_{n+m} / √5 | Match?\n";
    cout << "  --|---|-----------|-------------|--------\n";

    for (int n = 2; n <= 5; n++) {
        for (int m = 2; m <= 5; m++) {
            long long prod = fib[n] * fib[m];
            double approx = fib[n+m] / SQRT5;
            bool match = abs(prod - approx) < 0.5;
            
            cout << "  " << setw(2) << n << " | "
                 << setw(1) << m << " | "
                 << setw(9) << prod << " | "
                 << setw(11) << fixed << setprecision(2) << approx << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // ENCODING 4: SELF-REF DECOMPOSITION
    // ============================================

    cout << "\n========================================\n";
    cout << "  ENCODING 4: SELF-REF DECOMPOSITION\n";
    cout << "========================================\n\n";

    cout << "  x | n | frac | F_{n-1} | F_n | φ^frac | F_{n-1}×φ^frac + F_n×φ^frac×φ\n";
    cout << "  --|---|------|---------|-----|--------|----------------------------------\n";

    for (double x : {5.0, 7.0, 35.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        double frac = log_phi_x - n;
        double phi_frac = pow(PHI, frac);
        double a = fib[n-1] * phi_frac;
        double b = fib[n] * phi_frac;
        double recovered = a + b * PHI;
        bool match = abs(recovered - x) < 0.01;
        
        cout << "  " << setw(3) << x << " | "
             << setw(1) << n << " | "
             << setw(4) << fixed << setprecision(4) << frac << " | "
             << setw(7) << fib[n-1] << " | "
             << setw(3) << fib[n] << " | "
             << setw(6) << phi_frac << " | "
             << setw(32) << recovered << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    return 0;
}
