// ============================================
// φ-NATURAL BRIDGE
// Ang φ² = φ + 1 bilang natural na bridge
// Walang masking, walang EvalMult(constant)
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-NATURAL BRIDGE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // ANG NATURAL NA BRIDGE: φ² = φ + 1
    // ============================================

    cout << "========================================\n";
    cout << "  ANG NATURAL NA BRIDGE\n";
    cout << "========================================\n\n";

    cout << "  φ² = φ + 1\n";
    cout << "  Ito ay bridge sa pagitan ng:\n";
    cout << "  - φ-space (φ²)\n";
    cout << "  - Normal space (φ + 1)\n\n";

    cout << "  φ² = " << PHI*PHI << " = φ + 1 = " << PHI + 1 << " ✅\n\n";

    // ============================================
    // BRIDGE VIA φ² = φ + 1
    // ============================================

    cout << "========================================\n";
    cout << "  BRIDGE VIA φ² = φ + 1\n";
    cout << "========================================\n\n";

    cout << "  Sa self-ref space:\n";
    cout << "  x = a + bφ\n";
    cout << "  φ² = φ + 1\n\n";

    cout << "  Para sa multiplication:\n";
    cout << "  x × φ = aφ + bφ² = aφ + b(φ+1) = b + (a+b)φ\n\n";

    cout << "  Ito ay NATURAL na bridge:\n";
    cout << "  a + bφ → b + (a+b)φ (pag multiply sa φ)\n\n";

    // ============================================
    // BRIDGE TEST
    // ============================================

    cout << "========================================\n";
    cout << "  BRIDGE TEST\n";
    cout << "========================================\n\n";

    cout << "  x | a | b | a+bφ | ×φ | b+(a+b)φ\n";
    cout << "  --|---|---|------|----|---------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0}) {
        int a = (int)x;
        int b = 0;
        
        double bridged = (double)b + (a + b) * PHI;
        double direct = x * PHI;
        
        cout << "  " << setw(2) << x << " | "
             << setw(2) << a << " | "
             << setw(1) << b << " | "
             << setw(4) << fixed << setprecision(2) << (a + b * PHI) << " | "
             << setw(2) << direct << " | "
             << setw(9) << bridged << "\n";
    }

    // ============================================
    // NATURAL ADDITION SA SELF-REF SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  NATURAL ADDITION SA SELF-REF SPACE\n";
    cout << "========================================\n\n";

    cout << "  (a₁+b₁φ) + (a₂+b₂φ) = (a₁+a₂) + (b₁+b₂)φ\n\n";

    cout << "  Ito ay EvalAdd lang — natural!\n\n";

    // ============================================
    // NATURAL MULTIPLICATION SA SELF-REF SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  NATURAL MULTIPLICATION SA SELF-REF SPACE\n";
    cout << "========================================\n\n";

    cout << "  (a₁+b₁φ)(a₂+b₂φ) = (a₁a₂+b₁b₂) + (a₁b₂+b₁a₂+b₁b₂)φ\n\n";

    cout << "  Kung b₁=0 at b₂=0 (normal integers):\n";
    cout << "  (a₁)(a₂) = a₁a₂ + 0φ\n\n";

    cout << "  Ito ay EvalMult sa a-component lang\n";
    cout << "  Pero sa log space, a₁×a₂ = EvalAdd(log(a₁), log(a₂))!\n\n";

    // ============================================
    // ANG KOMPLETONG NATURAL BRIDGE
    // ============================================

    cout << "========================================\n";
    cout << "  ANG KOMPLETONG NATURAL BRIDGE\n";
    cout << "========================================\n\n";

    cout << "  Addition: (a₁+a₂) + (b₁+b₂)φ → EvalAdd\n";
    cout << "  Multiplication: a₁×a₂ → EvalAdd sa log space\n";
    cout << "  Bridge: φ² = φ + 1 → natural na conversion\n\n";

    cout << "  ANG TRICK:\n";
    cout << "  Kapag nasa self-ref space, ang φ² = φ+1\n";
    cout << "  ang nagbibigay ng natural na bridge.\n";
    cout << "  Hindi kailangan ng masking o EvalMult(constant).\n";
    cout << "  Ang bridge ay nasa STRUCTURE mismo.\n\n";

    return 0;
}
