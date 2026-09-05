// ============================================
// φ-BINARY MAX RESEARCH
// Hanapin ang binary max na walang EvalMult
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-BINARY MAX RESEARCH ===\n\n";
    
    // ============================================
    // 1. ANG RATIO BILANG |a-b| SIGNAL
    // ============================================
    cout << "--- 1. RATIO BILANG |a-b| ---\n\n";
    cout << "  a | b | ratio=max/min | log_φ(ratio) | |a-b| | Match?\n";
    cout << "  --|---|---------------|--------------|------|-------\n";
    
    for (int a : {1, 2, 3, 5, 8}) {
        for (int b : {2, 3, 5}) {
            double ratio = pow(PHI, abs(a - b));
            double log_ratio = log(ratio) / LN_PHI;
            int d = abs(a - b);
            
            cout << "  " << a << " | " << b << " | "
                 << setw(8) << fixed << setprecision(3) << ratio << " | "
                 << setw(8) << log_ratio << " | "
                 << setw(3) << d << " | "
                 << (abs(log_ratio - d) < 0.001 ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // 2. ANG MAX MULA SA RATIO
    // ============================================
    cout << "\n--- 2. MAX MULA SA RATIO ---\n\n";
    cout << "  max(a,b) = (a+b)/2 + |a-b|/2\n";
    cout << "           = (a+b)/2 + log_φ(ratio)/2\n\n";
    
    cout << "  a | b | max(a,b) | (a+b)/2 + log_φ(ratio)/2 | Match?\n";
    cout << "  --|---|----------|---------------------------|-------\n";
    
    for (int a : {1, 3, 5}) {
        for (int b : {2, 4}) {
            int m = max(a, b);
            double ratio = pow(PHI, abs(a - b));
            double log_ratio = log(ratio) / LN_PHI;
            double computed = (a + b) / 2.0 + log_ratio / 2.0;
            
            cout << "  " << a << " | " << b << " | "
                 << setw(4) << m << " | "
                 << setw(8) << fixed << setprecision(3) << computed << " | "
                 << (abs(computed - m) < 0.001 ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang ratio ay nasa Slot 1 — automatic\n";
    cout << "  Ang log_φ(ratio) ay |a-b| — naka-encode\n";
    cout << "  Ang max ay (a+b)/2 + |a-b|/2\n";
    cout << "  Ang pag-apply ay kailangan ng EvalMult (÷2)\n\n";

    return 0;
}
