// ============================================
// φ-NATURAL ROTATION — Emergent na Pattern
// Ang φ-power sequence ay may natural na cyclic
// pattern na parang rotation — walang EvalRotate
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "=== φ-NATURAL ROTATION ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Ang golden angle bilang natural rotation
    // ============================================
    cout << "--- 1. Golden angle bilang rotation ---\n\n";
    cout << "  2π/φ² = " << 2.0 * M_PI / (PHI * PHI) << " rad\n";
    cout << "         = " << 2.0 * M_PI / (PHI * PHI) * 180.0 / M_PI << " deg\n\n";

    // ============================================
    // 2. Ang φ-power bilang cyclic index
    // ============================================
    cout << "--- 2. φ-power bilang cyclic index ---\n\n";
    cout << "  n | φ^n mod 8 | floor(φ^n) mod 8\n";
    cout << "  --|-----------|------------------\n";
    
    for (int n = 0; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        double mod_8 = fmod(phi_n, 8.0);
        double floor_mod8 = fmod(floor(phi_n), 8.0);
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << mod_8 << " | "
             << setw(10) << floor_mod8 << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang Fibonacci rotation
    // ============================================
    cout << "--- 3. Fibonacci rotation ---\n\n";
    cout << "  Ang Fibonacci numbers ay may natural na\n";
    cout << "  rotation sa mod space\n\n";
    
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    cout << "  n | F_n | F_n mod 8 | rotation index\n";
    cout << "  --|-----|-----------|---------------\n";
    
    for (int n = 0; n <= 15; n++) {
        long long mod8 = fib[n] % 8;
        
        cout << "  " << setw(2) << n << " | "
             << setw(5) << fib[n] << " | "
             << setw(10) << mod8 << " | "
             << setw(10) << mod8 << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang natural na cyclic pattern
    // ============================================
    cout << "--- 4. Natural na cyclic pattern ---\n\n";
    cout << "  Ang φ-power ay may natural na periodicity:\n";
    cout << "  φ^(n+φ) = φ^n × φ^φ — period φ sa exponent\n\n";

    // ============================================
    // 5. Ang natural rotation para sa Rule 110
    // ============================================
    cout << "--- 5. Natural rotation para sa Rule 110 ---\n\n";
    cout << "  Ang Fibonacci mod 8 ay nagbibigay ng:\n";
    cout << "  natural na slot rotation na walang EvalRotate\n\n";

    cout << "  n | F_n mod 8 | Next F_{n+1} mod 8\n";
    cout << "  --|-----------|--------------------\n";
    
    for (int n = 0; n <= 15; n++) {
        long long curr = fib[n] % 8;
        long long next = fib[n+1] % 8;
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << curr << " | "
             << setw(10) << next << "\n";
    }
    cout << "\n";

    return 0;
}
