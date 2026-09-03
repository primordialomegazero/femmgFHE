// ============================================
// φ-TWO MODE
// Eksaktong bridge: F_n = φ×q + (0 o φ⁻¹)
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

    cout << "=== φ-TWO MODE ===\n\n";
    cout << "  n | F_n | q | φ×q | r | log_φ(F_n) | 1+log_φ(q) | Diff\n";
    cout << "  --|-----|---|-----|---|------------|------------|------\n";
    
    for (int n = 5; n <= 15; n++) {
        double F = (double)fib[n];
        double r = fmod(F, PHI);
        double q = floor(F / PHI);
        double log_F = log(F) / LN_PHI;
        double log_q = 1.0 + log(q) / LN_PHI;
        double diff = log_F - log_q;
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << F << " | "
             << setw(4) << fixed << setprecision(0) << q << " | "
             << setw(5) << (PHI * q) << " | "
             << setw(4) << r << " | "
             << setw(8) << log_F << " | "
             << setw(10) << log_q << " | "
             << setw(8) << diff << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Kung ang Diff ay maliit,\n";
    cout << "  log_φ(F_n) ≈ 1 + log_φ(q)\n";
    cout << "  Ang transition ay additive constant\n\n";

    return 0;
}
