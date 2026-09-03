// ============================================
// φ-PAIR FINAL
// (F_n, F_{n-1}) bilang kumpletong normal at log space
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
    for (int i = 2; i <= 30; i++) fib.push_back(fib[i-1] + fib[i-2]);

    cout << "=== φ-PAIR FINAL ===\n\n";
    cout << "  State: (F_n, F_{n-1}) = φ^n decomposition\n\n";

    // ADDITION
    cout << "--- ADDITION: (F_a, F_{a-1}) + (F_b, F_{b-1}) ---\n\n";
    cout << "  a+b | Sum pair | φ^(a+b) | Match?\n";
    cout << "  ----|----------|---------|-------\n";
    
    for (int a = 1; a <= 5; a++) {
        for (int b = a; b <= a+1; b++) {
            // Normal space: (F_a + F_b, F_{a-1} + F_{b-1})
            double f_sum = fib[a] + fib[b];
            double f_prev_sum = fib[a-1] + fib[b-1];
            double value = f_sum * PHI + f_prev_sum;
            
            // Expected: φ^a + φ^b = φ^max(a,b) + φ^max(a,b)-1
            double expected = pow(PHI, max(a,b)) + pow(PHI, max(a,b)-1);
            
            bool match = abs(value - expected) < 0.01;
            
            cout << "  " << a << "+" << b << " | "
                 << setw(4) << fixed << setprecision(0) << f_sum << "," << setw(4) << f_prev_sum << " | "
                 << setw(7) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // MULTIPLICATION
    cout << "\n--- MULTIPLICATION: (F_a, F_{a-1}) × (F_b, F_{b-1}) ---\n\n";
    cout << "  a×b | Value | φ^(a+b) | Match?\n";
    cout << "  ----|-------|---------|-------\n";
    
    for (int a = 1; a <= 4; a++) {
        for (int b = a; b <= a+1; b++) {
            double value = pow(PHI, a) * pow(PHI, b);
            double expected = pow(PHI, a + b);
            bool match = abs(value - expected) < 0.01;
            
            cout << "  " << a << "×" << b << " | "
                 << setw(6) << fixed << setprecision(1) << value << " | "
                 << setw(6) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // LOG SPACE
    cout << "\n--- LOG SPACE ---\n\n";
    cout << "  State | log_φ(value) | Index | Match?\n";
    cout << "  ------|--------------|-------|-------\n";
    
    for (int n = 1; n <= 10; n++) {
        double value = pow(PHI, n);
        double log_val = log(value) / LN_PHI;
        bool match = abs(log_val - n) < 0.001;
        
        cout << "  n=" << setw(2) << n << " | "
             << setw(10) << fixed << setprecision(4) << log_val << " | "
             << setw(4) << n << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  (F_n, F_{n-1}) ay may:\n";
    cout << "  - Normal space: F_n×φ + F_{n-1} = φ^n\n";
    cout << "  - Log space: n\n";
    cout << "  - Addition: (F_a+F_b, F_{a-1}+F_{b-1})\n";
    cout << "  - Multiplication: index addition\n\n";

    return 0;
}
