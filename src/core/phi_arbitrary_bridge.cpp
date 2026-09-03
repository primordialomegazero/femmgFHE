// ============================================
// φ-ARBITRARY BRIDGE
// Para sa arbitrary values, gamit ang
// log_φ(F) = log_φ(q×φ + r)
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

    cout << "=== φ-ARBITRARY BRIDGE ===\n\n";
    cout << "  Para sa arbitrary F, ang log_φ(F) ay:\n";
    cout << "  log_φ(F) = log_φ(q×φ + r)\n";
    cout << "           = log_φ(q×φ × (1 + r/(q×φ)))\n";
    cout << "           = log_φ(q) + 1 + log_φ(1 + r/(q×φ))\n\n";
    
    cout << "  F | q | r | log_φ(F) exact | log_φ(q)+1 | Correction\n";
    cout << "  --|---|---|----------------|------------|-----------\n";
    
    for (int n : {3, 4, 5, 6, 7, 8}) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double log_F = log(F) / LN_PHI;
        double log_q_plus_1 = log(q) / LN_PHI + 1.0;
        double corr = log(1.0 + r / (q * PHI)) / LN_PHI;
        
        cout << "  " << setw(2) << F << " | "
             << setw(2) << fixed << setprecision(0) << q << " | "
             << setw(4) << r << " | "
             << setw(10) << log_F << " | "
             << setw(10) << log_q_plus_1 << " | "
             << setw(10) << corr << "\n";
    }
    
    // Para sa arbitrary 7
    cout << "\n  Para sa F = 7:\n";
    {
        double F = 7.0;
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double log_F = log(F) / LN_PHI;
        double log_q_plus_1 = log(q) / LN_PHI + 1.0;
        double corr = log(1.0 + r / (q * PHI)) / LN_PHI;
        double total = log_q_plus_1 + corr;
        
        cout << "  q = " << q << ", r = " << r << "\n";
        cout << "  log_φ(7) exact: " << log_F << "\n";
        cout << "  log_φ(7) via bridge: " << total << "\n";
        cout << "  Error: " << (log_F - total) << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang correction term ay log_φ(1 + r/(q×φ))\n";
    cout << "  Para sa malaking q, ito ay ≈ r/(q×φ)\n\n";

    return 0;
}
