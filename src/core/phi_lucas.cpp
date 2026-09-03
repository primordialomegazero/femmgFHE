// ============================================
// φ-LUCAS
// Lucas numbers bilang natural na bridge
// log_φ(L_n) ≈ n (walang √5 offset)
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 50; i++) {
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    cout << "=== φ-LUCAS ===\n\n";
    cout << "  Lucas: 2, 1, 3, 4, 7, 11, 18, 29, 47, 76...\n";
    cout << "  Formula: L_n = φ^n + (-φ)^(-n)\n\n";
    
    cout << "  n | L_n | log_φ(L_n) | n | Diff\n";
    cout << "  --|-----|------------|---|------\n";
    
    for (int n = 2; n <= 20; n++) {
        double L = (double)lucas[n];
        double log_L = log(L) / LN_PHI;
        double diff = log_L - n;
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << lucas[n] << " | "
             << setw(10) << fixed << setprecision(4) << log_L << " | "
             << setw(3) << n << " | "
             << setw(8) << diff << "\n";
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  Ang diff ay nagco-converge sa 0\n";
    cout << "  log_φ(L_n) ≈ n para sa malaking n\n";
    cout << "  WALANG √5 offset — mas simple!\n\n";

    return 0;
}
