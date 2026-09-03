// ============================================
// φ-BRIDGE CONSTANT
// I-verify kung ang (r+q) ay F_n/φ
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 100; i++) fib.push_back(fib[i-1] + fib[i-2]);

    cout << "=== φ-BRIDGE CONSTANT ===\n\n";
    cout << "  n | F_n | r+q | F_n/φ | (r+q) - F_n/φ | Diff\n";
    cout << "  --|-----|-----|-------|---------------|------\n";
    
    for (int n = 3; n <= 20; n++) {
        double F = (double)fib[n];
        double r = fmod(F, PHI);
        double q = floor(F / PHI);
        double r_plus_q = r + q;
        double F_over_phi = F / PHI;
        double diff = r_plus_q - F_over_phi;
        
        cout << "  " << setw(2) << n << " | "
             << setw(6) << F << " | "
             << setw(6) << fixed << setprecision(3) << r_plus_q << " | "
             << setw(6) << F_over_phi << " | "
             << setw(10) << diff << " | "
             << setw(6) << (abs(diff) < 0.1 ? "✅" : "❌") << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Kung ang diff ay maliit, ang bridge ay:\n";
    cout << "  log_φ(F_n) = log_φ(r+q) + 1\n";
    cout << "  Ito ay KONSTANTE, hindi series\n\n";

    return 0;
}
