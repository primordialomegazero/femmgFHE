// ============================================
// φ-LUCAS-FIB BRIDGE
// Encoding: [x, F_n, L_n, L_n+F_n, L_n-F_n]
// Hanapin ang natural na bridge
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
    cout << "  φ-LUCAS-FIB BRIDGE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 30; i++) {
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    cout << "  x | n | F_n | L_n | L_n+F_n | L_n-F_n | L_n/F_n\n";
    cout << "  --|---|-----|-----|---------|---------|-------\n";

    for (double x : {5.0, 7.0, 35.0, 3.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        
        cout << "  " << setw(3) << x << " | "
             << setw(1) << n << " | "
             << setw(3) << fib[n] << " | "
             << setw(3) << lucas[n] << " | "
             << setw(7) << (lucas[n] + fib[n]) << " | "
             << setw(7) << (lucas[n] - fib[n]) << " | "
             << setw(5) << fixed << setprecision(2) << (double)lucas[n] / fib[n] << "\n";
    }

    // ============================================
    // ADDITION TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  ADDITION TEST\n";
    cout << "========================================\n\n";

    // 5: n=3, F_3=2, L_3=4
    // 7: n=4, F_4=3, L_4=7
    // 35: n=7, F_7=13, L_7=29

    // EvalAdd simulation
    double F_sum = fib[3] + fib[4];  // 2 + 3 = 5
    double L_sum = lucas[3] + lucas[4];  // 4 + 7 = 11
    
    cout << "  F(5) + F(7) = " << F_sum << "\n";
    cout << "  L(5) + L(7) = " << L_sum << "\n";
    cout << "  F(35) = " << fib[7] << "\n";
    cout << "  L(35) = " << lucas[7] << "\n\n";

    // ============================================
    // LUCAS + FIBONACCI IDENTITIES
    // ============================================

    cout << "========================================\n";
    cout << "  LUCAS + FIBONACCI IDENTITIES\n";
    cout << "========================================\n\n";

    cout << "  n | L_n = F_{n-1} + F_{n+1}\n";
    cout << "  --|------------------------\n";

    for (int n = 2; n <= 10; n++) {
        long long lhs = lucas[n];
        long long rhs = fib[n-1] + fib[n+1];
        bool match = (lhs == rhs);
        
        cout << "  " << setw(2) << n << " | "
             << setw(20) << lhs << " | "
             << setw(2) << rhs << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  n | L_n² - 5F_n² = ±4\n";
    cout << "  --|------------------\n";

    for (int n = 2; n <= 10; n++) {
        long long val = lucas[n] * lucas[n] - 5 * fib[n] * fib[n];
        
        cout << "  " << setw(2) << n << " | "
             << setw(16) << val << "\n";
    }

    return 0;
}
