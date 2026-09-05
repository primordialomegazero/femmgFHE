// ============================================
// φ-MAX NO DIVISION
// Max sa n-space na walang ÷2
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-MAX NO DIVISION ===\n\n";
    
    // Sa halip na max(a,b) = (a+b)/2 + |a-b|/2
    // gamitin ang:
    // 2×max(a,b) = (a+b) + |a-b|
    //
    // Ang 2×max ay walang division!
    
    cout << "  a | b | 2×max | a+b+|a-b| | Match?\n";
    cout << "  --|---|-------|-----------|-------\n";
    
    for (int a : {1, 3, 5, 8}) {
        for (int b : {2, 4, 7}) {
            int max_2x = 2 * max(a, b);
            int computed = a + b + abs(a - b);
            
            cout << "  " << a << " | " << b << " | "
                 << setw(4) << max_2x << " | "
                 << setw(6) << computed << " | "
                 << (max_2x == computed ? "✅" : "❌") << "\n";
        }
    }
    
    cout << "\n=== KEY ===\n";
    cout << "  2×max(a,b) = a + b + |a-b|\n";
    cout << "  Walang division!\n";
    cout << "  Ang |a-b| ay nasa Slot 1 (ratio)\n\n";
    
    // ============================================
    // ANG |a-b| SA RATIO
    // ============================================
    cout << "=== |a-b| SA RATIO ===\n\n";
    cout << "  Ang |a-b| = log_φ(max(F₁,F₂)/min(F₁,F₂))\n";
    cout << "  Ito ay nasa Slot 1 — automatic\n\n";
    
    cout << "  F₁ | F₂ | ratio | log_φ(ratio) = |a-b|\n";
    cout << "  ----|----|-------|------------------\n";
    
    for (double F1 : {3.0, 7.0, 13.0}) {
        for (double F2 : {5.0, 8.0}) {
            double n1 = log(F1) / LN_PHI;
            double n2 = log(F2) / LN_PHI;
            double ratio = pow(PHI, abs(n1 - n2));
            double log_ratio = log(ratio) / LN_PHI;
            double d = abs(n1 - n2);
            
            cout << "  " << setw(3) << F1 << " | "
                 << setw(3) << F2 << " | "
                 << setw(7) << fixed << setprecision(3) << ratio << " | "
                 << setw(7) << log_ratio << " ("
                 << setw(6) << d << ")\n";
        }
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang 2×max ay walang division\n";
    cout << "  Ang |a-b| ay automatic sa Slot 1\n";
    cout << "  Ang max ay: (2×max)/2 — pero ang ÷2 ay\n";
    cout << "  shift sa n-space (EvalAdd ng constant)\n\n";

    return 0;
}
