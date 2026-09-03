// ============================================
// φ-INDEX TO FIBONACCI
// I-convert ang floor_index papuntang F_n
// gamit ang φ^n / √5
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
    cout << "  φ-INDEX TO FIBONACCI\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double SQRT5 = sqrt(5.0);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // TEST: F_n ≈ φ^n / √5
    // ============================================

    cout << "========================================\n";
    cout << "  F_n ≈ φ^n / √5\n";
    cout << "========================================\n\n";

    cout << "  n | φ^n | φ^n/√5 | F_n | Match?\n";
    cout << "  --|-----|--------|-----|--------\n";

    for (int n = 0; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double approx = phi_n / SQRT5;
        long long f_n = fib[n];
        bool match = abs(approx - f_n) < 0.01;
        
        cout << "  " << setw(2) << n << " | "
             << setw(7) << fixed << setprecision(4) << phi_n << " | "
             << setw(6) << approx << " | "
             << setw(5) << f_n << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST: INDEX 7 → F_7
    // ============================================

    cout << "\n========================================\n";
    cout << "  INDEX 7 → F_7\n";
    cout << "========================================\n\n";

    int index = 7;
    double phi_7 = pow(PHI, index);
    double approx = phi_7 / SQRT5;
    long long f_7 = round(approx);

    cout << "  φ^7 = " << phi_7 << "\n";
    cout << "  φ^7/√5 = " << approx << "\n";
    cout << "  round(φ^7/√5) = " << f_7 << "\n";
    cout << "  Expected: F_7 = " << fib[7] << "\n";
    cout << "  Match: " << (f_7 == fib[7] ? "✅" : "❌") << "\n\n";

    // ============================================
    // ENCODING: [a, φ^n/√5, a+bφ, floor_index]
    // ============================================

    cout << "========================================\n";
    cout << "  BAGONG ENCODING\n";
    cout << "========================================\n\n";

    cout << "  Slot 0: a\n";
    cout << "  Slot 1: φ^n / √5 ≈ F_n\n";
    cout << "  Slot 2: a + bφ\n";
    cout << "  Slot 3: n (floor_index)\n\n";

    // Para sa 35:
    // floor = 7, F_7 = 13
    // a = 35 - 13φ = 13.9656
    // Slot 1: φ^7/√5 = 13.03
    
    double floor_35 = 7;
    double phi_7_val = pow(PHI, 7) / SQRT5;
    double a_35 = 35.0 - 13.0 * PHI;

    cout << "  35:\n";
    cout << "  Slot 0: " << a_35 << "\n";
    cout << "  Slot 1: " << phi_7_val << " ≈ 13\n";
    cout << "  Slot 2: " << (a_35 + 13.0 * PHI) << "\n";
    cout << "  Slot 3: " << floor_35 << "\n\n";

    return 0;
}
