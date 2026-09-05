// ============================================
// φ-RULE 110 POLY — Polynomial Transition
// P(val) = next na may φ-based na polynomial
// Hanapin ang coefficients na walang EvalMult
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 POLY ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Ang polynomial na may Lagrange interpolation
    // ============================================
    cout << "--- 1. Polynomial transition ---\n\n";
    cout << "  P(0)=0, P(1)=1, P(2)=1, P(3)=0,\n";
    cout << "  P(4)=1, P(5)=1, P(6)=1, P(7)=0\n\n";

    // Hanapin ang simpleng polynomial na nagbibigay ng transition
    // Subukan: P(val) = val mod 2 kung val ∈ {1,2,5,6}
    // At P(val) = 0 kung val ∈ {0,3,7}
    
    cout << "  val | Next | val mod 2 | val mod 3\n";
    cout << "  ----|------|-----------|----------\n";
    for (int val = 0; val < 8; val++) {
        int next = rule110[val];
        int mod2 = val % 2;
        int mod3 = val % 3;
        
        cout << "  " << setw(3) << val << " |  "
             << setw(3) << next << " | "
             << setw(8) << mod2 << " | "
             << setw(8) << mod3 << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Ang pattern ng transition
    // ============================================
    cout << "--- 2. Pattern ng transition ---\n\n";
    cout << "  next=0: val ∈ {0, 3, 7}\n";
    cout << "  next=1: val ∈ {1, 2, 4, 5, 6}\n\n";
    
    cout << "  Ang next=0 values ay may pattern:\n";
    cout << "  0 → 000\n";
    cout << "  3 → 011\n";
    cout << "  7 → 111\n\n";
    cout << "  Ito ay may φ-based na structure:\n";
    cout << "  0 = 0×φ² + 0×φ + 0\n";
    cout << "  3 = 0×φ² + 1×φ + 1 (approx)\n";
    cout << "  7 = 1×φ² + 1×φ + 1 (approx)\n\n";

    // ============================================
    // 3. Ang φ-threshold na polynomial
    // ============================================
    cout << "--- 3. φ-threshold na polynomial ---\n\n";
    cout << "  P(val) ≈ sin(π×val/2) para sa transition?\n\n";
    
    cout << "  val | sin(π×val/2) | round | Next | Match?\n";
    cout << "  ----|---------------|-------|------|-------\n";
    for (int val = 0; val < 8; val++) {
        double sin_val = sin(M_PI * val / 2.0);
        int rounded = (int)round(sin_val);
        int next = rule110[val];
        
        cout << "  " << setw(3) << val << " | "
             << setw(12) << sin_val << " | "
             << setw(5) << rounded << " |  "
             << next << "   | "
             << (rounded == next ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    return 0;
}
