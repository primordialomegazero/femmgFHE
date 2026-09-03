// ============================================
// φ-BRIDGE ODD/EVEN
// log_φ(F_n) = log_φ(q) + 1 (odd n)
// log_φ(F_n) = log_φ(q) + 1 + δ_n (even n)
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

    cout << "=== φ-BRIDGE ODD/EVEN ===\n\n";
    cout << "  n | F_n | q | log_φ(q) | log_φ(F_n) | Diff | δ_n\n";
    cout << "  --|-----|---|----------|------------|------|-----\n";
    
    vector<double> deltas(100, 0.0);
    
    for (int n = 5; n <= 20; n++) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double log_q = log(q) / LN_PHI;
        double log_F = log(F) / LN_PHI;
        double diff = log_F - log_q;
        double delta = diff - 1.0;
        deltas[n] = delta;
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << F << " | "
             << setw(3) << fixed << setprecision(0) << q << " | "
             << setw(8) << log_q << " | "
             << setw(10) << log_F << " | "
             << setw(5) << diff << " | "
             << setw(6) << delta << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  δ_n ≈ 0 para sa odd n\n";
    cout << "  δ_n ≈ φ^(-2n) para sa even n (maliit)\n\n";
    
    // Test ang δ_n sa FHE
    cout << "--- TEST: 7 × 3 = 21 gamit ang bridge ---\n\n";
    {
        double F_7 = 7.0;
        double q_7 = floor(F_7 / PHI);  // = 4
        double log_q_7 = log(q_7) / LN_PHI;
        double log_F_7 = log(F_7) / LN_PHI;
        double bridge_7 = log_q_7 + 1.0;  // = log_φ(4) + 1
        
        cout << "  F = 7, q = " << q_7 << "\n";
        cout << "  log_φ(7) exact: " << log_F_7 << "\n";
        cout << "  log_φ(7) bridge: " << bridge_7 << "\n";
        cout << "  Error: " << (log_F_7 - bridge_7) << "\n\n";
        
        // Multiplication: 7 × 3
        double log_3 = log(3.0) / LN_PHI;
        double log_21_bridge = bridge_7 + log_3;
        double result = pow(PHI, log_21_bridge);
        cout << "  Result via bridge: " << result << " (Expected: 21)\n";
        cout << "  Match: " << (abs(result - 21.0) < 0.5 ? "✅" : "❌") << "\n";
    }

    return 0;
}
