// ============================================
// φ-EMERGENT TRANSITION
// Gamit ang r+q o r×q bilang bridge
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

    cout << "=== φ-EMERGENT TRANSITION ===\n\n";
    
    cout << "  n | F_n | r+q | (r+q)/F_n | r×q | log_φ(r+q) | log_φ(F_n)\n";
    cout << "  --|-----|-----|-----------|-----|------------|-----------\n";
    
    for (int n = 3; n <= 12; n++) {
        double F = (double)fib[n];
        double r = fmod(F, PHI);
        double q = floor(F / PHI);
        double r_plus_q = r + q;
        double r_times_q = r * q;
        double log_r_plus_q = log(r_plus_q) / LN_PHI;
        double log_F = log(F) / LN_PHI;
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << F << " | "
             << setw(5) << fixed << setprecision(2) << r_plus_q << " | "
             << setw(8) << (r_plus_q / F) << " | "
             << setw(5) << r_times_q << " | "
             << setw(9) << log_r_plus_q << " | "
             << setw(10) << log_F << "\n";
    }
    
    cout << "\n=== HYPOTHESIS ===\n";
    cout << "  Kung ang (r+q) ay may simpleng relasyon\n";
    cout << "  sa F_n o sa φ, maaari itong gamitin\n";
    cout << "  bilang transition bridge\n\n";

    return 0;
}
