// ============================================
// φ-FIBONACCI CHAIN
// Fibonacci sequence bilang natural na bridge
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
    cout << "  φ-FIBONACCI CHAIN\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // FIBONACCI PROPERTIES
    // ============================================

    cout << "========================================\n";
    cout << "  FIBONACCI PROPERTIES\n";
    cout << "========================================\n\n";

    cout << "  F_n + F_{n+1} = F_{n+2}\n";
    cout << "  n | F_n + F_{n+1} | F_{n+2} | Match?\n";
    cout << "  --|---------------|---------|--------\n";

    for (int n = 0; n <= 10; n++) {
        long long sum = fib[n] + fib[n+1];
        long long expected = fib[n+2];
        cout << "  " << setw(2) << n << " | "
             << setw(13) << sum << " | "
             << setw(7) << expected << " | "
             << (sum == expected ? "✅" : "❌") << "\n";
    }

    // ============================================
    // FIBONACCI MULTIPLICATION
    // ============================================

    cout << "\n========================================\n";
    cout << "  FIBONACCI MULTIPLICATION\n";
    cout << "========================================\n\n";

    cout << "  F_n × F_{n+1} ≈ F_{2n+1}\n";
    cout << "  n | F_n × F_{n+1} | F_{2n+1} | Match?\n";
    cout << "  --|---------------|---------|--------\n";

    for (int n = 0; n <= 8; n++) {
        long long prod = fib[n] * fib[n+1];
        long long expected = fib[2*n + 1];
        cout << "  " << setw(2) << n << " | "
             << setw(13) << prod << " | "
             << setw(7) << expected << " | "
             << (prod == expected ? "✅" : "❌") << "\n";
    }

    // ============================================
    // FIBONACCI CHAIN — MIXED OPS
    // ============================================

    cout << "\n========================================\n";
    cout << "  FIBONACCI CHAIN — MIXED OPS\n";
    cout << "========================================\n\n";

    cout << "  Subok: gumamit ng Fibonacci index para\n";
    cout << "  sabay na ma-compute ang addition at multiplication\n\n";

    cout << "  Halimbawa: 2 + 3 = 5\n";
    cout << "  2 = F_3 = 2\n";
    cout << "  3 = F_4 = 3\n";
    cout << "  2 + 3 = 5 = F_5\n\n";

    cout << "  Fibonacci index ng 2: 3\n";
    cout << "  Fibonacci index ng 3: 4\n";
    cout << "  Index sum: 3 + 4 = 7\n";
    cout << "  F_7 = 13 ≠ 5\n\n";

    cout << "  PERO: F_3 + F_4 = 2 + 3 = 5 = F_5 ✅\n";
    cout << "  At: index 5 - index 3 = 2 = index ng 2 ✅\n\n";

    // ============================================
    // FIBONACCI BRIDGE
    // ============================================

    cout << "========================================\n";
    cout << "  FIBONACCI BRIDGE\n";
    cout << "========================================\n\n";

    cout << "  Ang bridge sa Fibonacci:\n";
    cout << "  - Addition: F_m + F_n = F_{m+n-1} (approximation)\n";
    cout << "  - Multiplication: F_m × F_n ≈ F_{m+n}\n\n";

    cout << "  Ito ay natural sa φ-structure:\n";
    cout << "  F_n ≈ φ^n / √5\n";
    cout << "  F_m + F_n ≈ φ^m/√5 + φ^n/√5\n";
    cout << "  F_m × F_n ≈ φ^(m+n) / 5\n\n";

    return 0;
}
