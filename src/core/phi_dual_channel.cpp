// ============================================
// φ-DUAL CHANNEL
// (mod φ, wrap count) bilang kumpletong state
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "=== φ-DUAL CHANNEL ===\n\n";
    cout << "  n | r_n (mod φ) | q_n (wrap) | Reconstruct F_n = q_n×φ + r_n | F_n actual | Match?\n";
    cout << "  --|-------------|-------------|--------------------------------|------------|-------\n";

    for (int n = 2; n <= 15; n++) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double recon = q * PHI + r;
        bool match = abs(recon - F) < 0.001;
        
        cout << "  " << setw(2) << n << " | "
             << setw(11) << fixed << setprecision(4) << r << " | "
             << setw(11) << q << " | "
             << setw(30) << recon << " | "
             << setw(10) << F << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Kung ang reconstruction ay exact,\n";
    cout << "  ang (r_n, q_n) ay kumpletong state\n";
    cout << "  na may parehong normal at log space\n\n";

    return 0;
}
