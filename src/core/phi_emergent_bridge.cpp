// ============================================
// φ-EMERGENT BRIDGE
// I-explore ang iba pang emergent properties
// na maaaring magbigay ng transition
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double LOG_SQRT5 = log(sqrt(5.0)) / LN_PHI;
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 100; i++) fib.push_back(fib[i-1] + fib[i-2]);

    cout << "=== φ-EMERGENT BRIDGE ===\n\n";

    // ============================================
    // 1. DUALITY: r_n at q_n bilang complementary
    // ============================================
    cout << "--- 1. DUALITY (r_n, q_n) ---\n\n";
    cout << "  n | r_n | q_n | r_n + q_n | r_n × q_n | r_n/q_n\n";
    cout << "  --|-----|-----|-----------|-----------|--------\n";
    for (int n = 3; n <= 12; n++) {
        double F = (double)fib[n];
        double r = fmod(F, PHI);
        double q = floor(F / PHI);
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fixed << setprecision(3) << r << " | "
             << setw(4) << q << " | "
             << setw(9) << (r + q) << " | "
             << setw(9) << (r * q) << " | "
             << setw(9) << (r / q) << "\n";
    }

    // ============================================
    // 2. SELF-REFERENCE: F_n sa φ terms
    // ============================================
    cout << "\n--- 2. SELF-REFERENCE ---\n\n";
    cout << "  n | F_n | φ^n / √5 | (F_n × √5) / φ^n\n";
    cout << "  --|-----|-----------|----------------\n";
    for (int n = 3; n <= 12; n++) {
        double F = (double)fib[n];
        double phi_n = pow(PHI, n) / sqrt(5.0);
        double ratio = F * sqrt(5.0) / pow(PHI, n);
        cout << "  " << setw(2) << n << " | "
             << setw(5) << F << " | "
             << setw(9) << fixed << setprecision(2) << phi_n << " | "
             << setw(12) << ratio << "\n";
    }

    // ============================================
    // 3. IRRATIONALITY: φ^(-1) at φ^(-2)
    // ============================================
    cout << "\n--- 3. IRRATIONALITY ---\n\n";
    cout << "  φ^(-1) = " << PHI - 1.0 << "\n";
    cout << "  φ^(-2) = " << 2.0 - PHI << "\n";
    cout << "  φ^(-3) = " << 2.0 * PHI - 3.0 << "\n";
    cout << "  φ^(-4) = " << 5.0 - 3.0 * PHI << "\n\n";
    
    cout << "  n | F_n mod φ | F_n mod φ² | F_n mod φ³\n";
    cout << "  --|-----------|------------|-----------\n";
    for (int n = 3; n <= 10; n++) {
        double F = (double)fib[n];
        cout << "  " << setw(2) << n << " | "
             << setw(9) << fixed << setprecision(4) << fmod(F, PHI) << " | "
             << setw(10) << fmod(F, PHI*PHI) << " | "
             << setw(10) << fmod(F, PHI*PHI*PHI) << "\n";
    }

    // ============================================
    // 4. PERIODICITY: F_n mod iba't ibang φ-powers
    // ============================================
    cout << "\n--- 4. PERIODICITY ---\n\n";
    cout << "  F_n mod φ² para sa n=3..15\n\n";
    for (int n = 3; n <= 15; n++) {
        double F = (double)fib[n];
        double mod_phi2 = fmod(F, PHI * PHI);
        cout << "  " << setw(2) << n << ": " 
             << setw(8) << fixed << setprecision(4) << mod_phi2 << "\n";
    }

    // ============================================
    // 5. CONVERGENCE: r_n papuntang 0 o φ⁻¹
    // ============================================
    cout << "\n--- 5. CONVERGENCE ---\n\n";
    cout << "  n | r_n | r_n - φ⁻¹ (even n) | r_n (odd n)\n";
    cout << "  --|-----|---------------------|-----------\n";
    for (int n = 5; n <= 15; n++) {
        double F = (double)fib[n];
        double r = fmod(F, PHI);
        cout << "  " << setw(2) << n << " | "
             << setw(7) << fixed << setprecision(4) << r;
        if (n % 2 == 0) {
            cout << " | " << setw(7) << (r - (PHI - 1.0));
        } else {
            cout << " | " << setw(7) << r;
        }
        cout << "\n";
    }

    cout << "\n=== KEY OBSERVATIONS ===\n";
    cout << "  Hanapin kung alin sa mga properties na ito\n";
    cout << "  ang nagbibigay ng pinakamalinaw na bridge\n\n";

    return 0;
}
