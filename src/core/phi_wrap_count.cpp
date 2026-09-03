// ============================================
// φ-WRAP COUNT
// Hanapin kung ang wrap count (q_n) ang
// natural na tulay sa pagitan ng normal at log space
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

    cout << "=== φ-WRAP COUNT ===\n\n";
    cout << "  n | F_n | q_n (wrap) | r_n (mod φ) | q_n - n | log_φ(F_n)\n";
    cout << "  --|-----|-------------|--------------|----------|-----------\n";

    for (int n = 2; n <= 15; n++) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double log_phi_F = log(F) / log(PHI);
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fib[n] << " | "
             << setw(11) << q << " | "
             << setw(12) << fixed << setprecision(4) << r << " | "
             << setw(8) << (q - n) << " | "
             << setw(10) << log_phi_F << "\n";
    }

    cout << "\n=== KEY OBSERVATION ===\n";
    cout << "  Kung q_n ≈ n - 1, ang wrap count ay log-space signal\n";
    cout << "  Kung ang (q_n, r_n) ay parehong naka-encode,\n";
    cout << "  mayroon kang kumpletong bridge\n\n";

    return 0;
}
