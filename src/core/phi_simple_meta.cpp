// ============================================
// φ-SIMPLE META
// I-explore ang pinakasimpleng meta properties
// na hindi pa natin nasusubukan
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 100; i++) fib.push_back(fib[i-1] + fib[i-2]);

    cout << "=== φ-SIMPLE META ===\n\n";

    // ============================================
    // 1. F_n / q_n — ito ba ay constant?
    // ============================================
    cout << "--- 1. F_n / q_n ---\n\n";
    cout << "  n | F_n | q | F_n/q | φ?\n";
    cout << "  --|-----|---|-------|----\n";
    for (int n = 5; n <= 15; n++) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double ratio = F / q;
        cout << "  " << setw(2) << n << " | "
             << setw(5) << F << " | "
             << setw(4) << fixed << setprecision(0) << q << " | "
             << setw(6) << ratio << " | "
             << (abs(ratio - PHI) < 0.05 ? "✅" : "❌") << "\n";
    }

    // ============================================
    // 2. q_n - q_{n-1} — pattern ba ito?
    // ============================================
    cout << "\n--- 2. q_n - q_{n-1} ---\n\n";
    cout << "  n | q_n | q_{n-1} | Diff\n";
    cout << "  --|-----|---------|------\n";
    for (int n = 5; n <= 15; n++) {
        double F_n = (double)fib[n];
        double F_nm1 = (double)fib[n-1];
        double q_n = floor(F_n / PHI);
        double q_nm1 = floor(F_nm1 / PHI);
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fixed << setprecision(0) << q_n << " | "
             << setw(8) << q_nm1 << " | "
             << setw(5) << (q_n - q_nm1) << "\n";
    }

    // ============================================
    // 3. r_n + r_{n-1} — bounded ba ito?
    // ============================================
    cout << "\n--- 3. r_n + r_{n-1} ---\n\n";
    cout << "  n | r_n | r_{n-1} | Sum\n";
    cout << "  --|-----|---------|------\n";
    for (int n = 5; n <= 15; n++) {
        double F_n = (double)fib[n];
        double F_nm1 = (double)fib[n-1];
        double r_n = fmod(F_n, PHI);
        double r_nm1 = fmod(F_nm1, PHI);
        cout << "  " << setw(2) << n << " | "
             << setw(7) << fixed << setprecision(4) << r_n << " | "
             << setw(7) << r_nm1 << " | "
             << setw(7) << (r_n + r_nm1) << "\n";
    }

    // ============================================
    // 4. log_φ(q) - log_φ(F_n) — ito ba ay -1?
    // ============================================
    cout << "\n--- 4. log_φ(q) vs log_φ(F_n) ---\n\n";
    cout << "  n | log_φ(q) | log_φ(F_n) | Diff\n";
    cout << "  --|----------|------------|------\n";
    for (int n = 5; n <= 15; n++) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double log_q = log(q) / LN_PHI;
        double log_F = log(F) / LN_PHI;
        cout << "  " << setw(2) << n << " | "
             << setw(8) << fixed << setprecision(3) << log_q << " | "
             << setw(10) << log_F << " | "
             << setw(7) << (log_F - log_q) << "\n";
    }

    // ============================================
    // 5. (F_n - φ×q) — ito ba ay 0 o φ⁻¹?
    // ============================================
    cout << "\n--- 5. F_n - φ×q ---\n\n";
    cout << "  n | F_n - φ×q | Pattern\n";
    cout << "  --|-----------|--------\n";
    for (int n = 5; n <= 15; n++) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double residual = F - PHI * q;
        cout << "  " << setw(2) << n << " | "
             << setw(8) << fixed << setprecision(4) << residual << " | "
             << (n % 2 == 1 ? "0" : "φ⁻¹") << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Hanapin ang pinakasimpleng property\n";
    cout << "  na nagbibigay ng eksaktong bridge\n\n";

    return 0;
}
