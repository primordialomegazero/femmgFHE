// ============================================
// φ-SELF-REF ADD V2
// Self-referential na min at abs
// gamit ang φ² = φ + 1
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
    cout << "  φ-SELF-REF ADD V2\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // SELF-REFERENTIAL ENCODING
    // ============================================

    cout << "========================================\n";
    cout << "  SELF-REFERENTIAL ENCODING\n";
    cout << "========================================\n\n";

    cout << "  I-encode ang value bilang:\n";
    cout << "  x = a + bφ (φ-basis)\n";
    cout << "  Kung saan a at b ay natural na magkakaugnay\n\n";

    cout << "  Ang self-referential property:\n";
    cout << "  φ² = φ + 1\n";
    cout << "  φ³ = 2φ + 1\n";
    cout << "  φ⁴ = 3φ + 2\n\n";

    cout << "  ANG TRICK:\n";
    cout << "  Kapag nag-EvalAdd tayo ng dalawang φ-basis values:\n";
    cout << "  (a₁ + b₁φ) + (a₂ + b₂φ) = (a₁+a₂) + (b₁+b₂)φ\n\n";

    cout << "  At ang φ² = φ+1 ay nagbibigay ng natural na:\n";
    cout << "  - Carry: b > 1 → carry sa a\n";
    cout << "  - Borrow: b < 0 → borrow mula sa a\n\n";

    // ============================================
    // TEST: 2 + 3 = 5
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: 2 + 3 = 5\n";
    cout << "========================================\n\n";

    cout << "  2 = 2 + 0φ\n";
    cout << "  3 = 3 + 0φ\n";
    cout << "  2 + 3 = 5 + 0φ = 5 ✅\n\n";

    // ============================================
    // TEST: φ + φ = ?
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: φ + φ = ?\n";
    cout << "========================================\n\n";

    cout << "  φ = 0 + 1φ\n";
    cout << "  φ + φ = 0 + 2φ\n";
    cout << "  Pero 2φ = φ + φ = φ² + φ - 1 = φ² + φ⁻¹\n\n";

    cout << "  Sa self-referential:\n";
    cout << "  2φ = φ² + φ - φ² + φ = φ² + (φ - φ²)\n";
    cout << "  = φ² + (φ - φ - 1)\n";
    cout << "  = φ² - 1\n\n";

    cout << "  Kaya: φ + φ = φ² - 1 = 2.618 - 1 = 1.618\n";
    cout << "  Mali! Dapat: φ + φ = 3.236\n\n";

    cout << "  HINDI GUMAGANA ANG SIMPLENG φ-BASIS\n\n";

    // ============================================
    // ANG TAMANG SELF-REFERENTIAL
    // ============================================

    cout << "========================================\n";
    cout << "  ANG TAMANG SELF-REFERENTIAL\n";
    cout << "========================================\n\n";

    cout << "  Ang tamang encoding ay φ-POWER:\n";
    cout << "  x = φ^n\n\n";

    cout << "  Addition: φ^a + φ^b\n";
    cout << "  Kung b = a-1: = φ^(a+1) ✅\n";
    cout << "  Kung b = a: = 2φ^a = φ^(a+log_φ(2))\n";
    cout << "  Kung b arbitrary: = φ^min × (1 + φ^diff)\n\n";

    cout << "  ANG SELF-REFERENTIAL MIN:\n";
    cout << "  φ^a + φ^b = φ^a × φ^b × (φ^(-a) + φ^(-b))\n";
    cout << "  = φ^(a+b) × (φ^(-a) + φ^(-b))\n";
    cout << "  = φ^(a+b) × φ^min(-a,-b) × (1 + φ^|a-b|)\n\n";

    cout << "  Sa log space:\n";
    cout << "  log_φ(a+b) = (a+b) + min(-a,-b) + log_φ(1 + φ^|a-b|)\n";
    cout << "  = max(a,b) + log_φ(1 + φ^|a-b|)\n\n";

    cout << "  ANG EMERGENT MIN/MAX:\n";
    cout << "  max(a,b) = (a+b + |a-b|) / 2\n";
    cout << "  min(a,b) = (a+b - |a-b|) / 2\n";
    cout << "  |a-b| = sqrt((a-b)²)\n\n";

    return 0;
}
