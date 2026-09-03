// ============================================
// φ-SELF-REFERENTIAL CONVERSION
// I-convert ang log_φ(x) papuntang x gamit
// ang Fibonacci decomposition a + bφ
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
    cout << "  φ-SELF-REFERENTIAL CONVERSION\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // Fibonacci sequence
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // CONVERSION: log_φ(x) → a + bφ
    // ============================================

    cout << "========================================\n";
    cout << "  CONVERSION: log_φ(x) → a + bφ\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | n | F_n (b) | a | a + bφ | Match?\n";
    cout << "  --|-----------|----|---------|-----|---------|--------\n";

    for (double x : {5.0, 8.0, 13.0, 21.0, 34.0, 35.0, 55.0, 89.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        
        // b = F_n (mula sa self-referential decomposition)
        long long b = fib[n];
        
        // a = x - b × φ
        double a = x - b * PHI;
        
        double recovered = a + b * PHI;
        bool match = abs(recovered - x) < 0.01;
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(2) << n << " | "
             << setw(7) << b << " | "
             << setw(4) << a << " | "
             << setw(7) << recovered << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST: (5 × 7) + 3 = 38
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST: (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    // Step 1: 5 × 7 = 35 sa log space
    double log_5 = log(5.0) / LN_PHI;
    double log_7 = log(7.0) / LN_PHI;
    double log_35 = log_5 + log_7;
    
    cout << "  log_φ(5) + log_φ(7) = " << log_35 << "\n\n";

    // Step 2: I-decompose ang 35 sa a + bφ
    int n_35 = (int)floor(log_35);
    long long b_35 = fib[n_35];
    double a_35 = 35.0 - b_35 * PHI;
    
    cout << "  35 = " << a_35 << " + " << b_35 << "φ\n";
    cout << "  = " << (a_35 + b_35 * PHI) << "\n\n";

    // Step 3: + 3
    double result = a_35 + 3.0;
    cout << "  " << a_35 << " + 3 = " << result << " (expected: 38)\n";
    cout << "  Match: " << (abs(result - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    return 0;
}
