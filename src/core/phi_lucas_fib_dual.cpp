// ============================================
// φ-LUCAS-FIB DUAL SPACE
// (L_n, F_n) bilang kumpletong representation
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double SQRT5 = sqrt(5.0);
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 30; i++) lucas.push_back(lucas[i-1] + lucas[i-2]);

    cout << "=== φ-LUCAS-FIB DUAL SPACE ===\n\n";
    cout << "  n | F_n | L_n | (L_n + F_n√5)/2 | φ^n | Match?\n";
    cout << "  --|-----|-----|-----------------|-----|-------\n";
    
    for (int n = 1; n <= 15; n++) {
        double recon = (lucas[n] + fib[n] * SQRT5) / 2.0;
        double phi_n = pow(PHI, n);
        
        cout << "  " << setw(2) << n << " | "
             << setw(4) << fib[n] << " | "
             << setw(4) << lucas[n] << " | "
             << setw(12) << fixed << setprecision(4) << recon << " | "
             << setw(7) << phi_n << " | "
             << (abs(recon - phi_n) < 0.01 ? "✅" : "❌") << "\n";
    }
    
    cout << "\n=== ARBITRARY DECOMPOSITION ===\n\n";
    cout << "  F | n (approx) | L_n + F_n√5 | Reconstructed | F | Match?\n";
    cout << "  --|------------|--------------|----------------|----|-------\n";
    
    for (double F : {3.0, 5.0, 7.0, 10.0, 13.0, 21.0, 34.0, 55.0, 100.0}) {
        // Hanapin ang n kung saan ang φ^n ≈ F
        double n_approx = log(F) / log(PHI);
        int n = (int)round(n_approx);
        
        double recon = (lucas[n] + fib[n] * SQRT5) / 2.0;
        
        cout << "  " << setw(5) << F << " | "
             << setw(6) << fixed << setprecision(1) << n_approx << " | "
             << setw(6) << lucas[n] << "+" << fib[n] << "√5 | "
             << setw(9) << recon << " | "
             << setw(5) << F << " | "
             << (abs(recon - F) < 1.0 ? "✅" : "❌") << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang (L_n, F_n) ay kumpletong decomposition\n";
    cout << "  ng φ^n — at maaaring gamitin para sa\n";
    cout << "  arbitrary values na may φ-structure\n\n";

    return 0;
}
