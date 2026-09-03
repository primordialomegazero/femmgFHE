// ============================================
// φ-BRIDGE EXACT
// Hanapin ang eksaktong relasyon sa pagitan
// ng F_n at (r+q)
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

    cout << "=== φ-BRIDGE EXACT ===\n\n";
    cout << "  n | F_n | r | q | φ×(r+q) | φ×(r+q) - F_n\n";
    cout << "  --|-----|---|----|---------|---------------\n";
    
    for (int n = 3; n <= 20; n++) {
        double F = (double)fib[n];
        double r = fmod(F, PHI);
        double q = floor(F / PHI);
        double phi_times = PHI * (r + q);
        double diff = phi_times - F;
        
        cout << "  " << setw(2) << n << " | "
             << setw(6) << F << " | "
             << setw(4) << fixed << setprecision(2) << r << " | "
             << setw(4) << q << " | "
             << setw(8) << phi_times << " | "
             << setw(10) << diff << "\n";
    }
    
    cout << "\n=== ALTERNATIVE: q na nag-iisa ===\n\n";
    cout << "  n | F_n | q | φ×q | F_n - φ×q | r\n";
    cout << "  --|-----|---|-----|----------|---\n";
    
    for (int n = 3; n <= 15; n++) {
        double F = (double)fib[n];
        double r = fmod(F, PHI);
        double q = floor(F / PHI);
        double phi_q = PHI * q;
        
        cout << "  " << setw(2) << n << " | "
             << setw(6) << F << " | "
             << setw(4) << fixed << setprecision(0) << q << " | "
             << setw(8) << phi_q << " | "
             << setw(10) << (F - phi_q) << " | "
             << setw(4) << r << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Kung F_n - φ×q = r,\n";
    cout << "  walang error sa decomposition\n\n";

    return 0;
}
