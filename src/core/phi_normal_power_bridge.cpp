// ============================================
// φ-NORMAL-POWER BRIDGE
// Ang bridge sa pagitan ng normal at power space
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
    cout << "  φ-NORMAL-POWER BRIDGE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // BRIDGE: x → φ^n
    // ============================================

    cout << "========================================\n";
    cout << "  NORMAL → POWER SPACE\n";
    cout << "========================================\n\n";

    cout << "  x | n = log_φ(x) | φ^n | Match?\n";
    cout << "  --|---------------|-----|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0}) {
        double n = log(x) / LN_PHI;
        double phi_n = pow(PHI, n);
        bool match = abs(phi_n - x) < 0.01;
        
        cout << "  " << setw(3) << x << " | "
             << setw(13) << fixed << setprecision(4) << n << " | "
             << setw(5) << phi_n << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // BRIDGE: φ^n → x
    // ============================================

    cout << "\n========================================\n";
    cout << "  POWER → NORMAL SPACE\n";
    cout << "========================================\n\n";

    cout << "  n | φ^n | x = φ^n | Match?\n";
    cout << "  --|-----|----------|--------\n";

    for (int n : {0, 1, 2, 3, 4, 5}) {
        double phi_n = pow(PHI, n);
        cout << "  " << setw(2) << n << " | "
             << setw(7) << fixed << setprecision(4) << phi_n << " | "
             << setw(8) << phi_n << " | ✅\n";
    }

    // ============================================
    // NATURAL BRIDGE PROPERTY
    // ============================================

    cout << "\n========================================\n";
    cout << "  NATURAL BRIDGE PROPERTY\n";
    cout << "========================================\n\n";

    cout << "  φ^n = F_{n-1} + F_n × φ\n";
    cout << "  (Fibonacci decomposition ang bridge)\n\n";

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  n | φ^n | F_{n-1} + F_n × φ | Match?\n";
    cout << "  --|-----|--------------------|--------\n";

    for (int n : {1, 2, 3, 4, 5, 6, 7, 8}) {
        double phi_n = pow(PHI, n);
        double fib_decomp = fib[n-1] + fib[n] * PHI;
        bool match = abs(phi_n - fib_decomp) < 0.001;
        
        cout << "  " << n << " | "
             << setw(7) << fixed << setprecision(4) << phi_n << " | "
             << setw(18) << fib_decomp << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // ANG BRIDGE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ANG BRIDGE\n";
    cout << "========================================\n\n";

    cout << "  Normal → Power: x → log_φ(x) = n\n";
    cout << "  Power → Normal: n → φ^n = F_{n-1} + F_n × φ\n\n";

    cout << "  Ang bridge ay:\n";
    cout << "  1. log_φ(x) para sa normal→power\n";
    cout << "  2. φ^n = F_{n-1} + F_nφ para sa power→normal\n\n";

    cout << "  Pareho ay pwedeng gawin sa EvalAdd at EvalSub!\n";
    cout << "  Ang log_φ(x) ay kailangan ng EvalMult(constant)\n";
    cout << "  Ang φ^n ay F_{n-1} + F_n × φ — EvalAdd at EvalMult(constant)\n\n";

    return 0;
}
