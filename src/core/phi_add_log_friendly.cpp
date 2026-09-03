// ============================================
// φ-ADD LOG FRIENDLY
// I-convert ang addition formula sa log space
// para lahat ay EvalAdd at EvalSub lang
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-ADD LOG FRIENDLY\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // LOG-FRIENDLY ADDITION
    // ============================================

    cout << "========================================\n";
    cout << "  LOG-FRIENDLY ADDITION\n";
    cout << "========================================\n\n";

    cout << "  φ^a + φ^b = φ^min(a,b) × (1 + φ^|a-b|)\n\n";

    cout << "  Sa log space:\n";
    cout << "  log_φ(φ^a + φ^b) = min(a,b) + log_φ(1 + φ^|a-b|)\n\n";

    cout << "  min(a,b) = (a+b - |a-b|) / 2\n";
    cout << "  |a-b| = sqrt((a-b)²)\n\n";

    // ============================================
    // LAHAT SA LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  LAHAT SA LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  Sa log space, ang |a-b| ay:\n";
    cout << "  log(|a-b|) = log(a-b) + log(a-b) - log(2)\n";
    cout << "  (kung a > b, |a-b| = a-b)\n\n";

    cout << "  At ang min(a,b) ay:\n";
    cout << "  min(a,b) = (a+b - |a-b|) / 2\n";
    cout << "  log(min) = log(a+b - |a-b|) - log(2)\n\n";

    // ============================================
    // SIMPLIFICATION
    // ============================================

    cout << "========================================\n";
    cout << "  SIMPLIFICATION\n";
    cout << "========================================\n\n";

    cout << "  φ^a + φ^b = φ^min(a,b) × (1 + φ^|a-b|)\n\n";

    cout << "  Kung a ≥ b:\n";
    cout << "  min = b, |a-b| = a-b\n";
    cout << "  φ^a + φ^b = φ^b × (1 + φ^(a-b))\n";
    cout << "  = φ^b + φ^(b+a-b)\n";
    cout << "  = φ^b + φ^a ✅\n\n";

    cout << "  Kung b ≥ a:\n";
    cout << "  min = a, |a-b| = b-a\n";
    cout << "  φ^a + φ^b = φ^a × (1 + φ^(b-a))\n";
    cout << "  = φ^a + φ^(a+b-a)\n";
    cout << "  = φ^a + φ^b ✅\n\n";

    cout << "  ANG FORMULA AY EXACT — identity lang!\n\n";

    // ============================================
    // ANG EMERGENT INSIGHT
    // ============================================

    cout << "========================================\n";
    cout << "  ANG EMERGENT INSIGHT\n";
    cout << "========================================\n\n";

    cout << "  φ^a + φ^b = φ^min(a,b) × (1 + φ^|a-b|)\n";
    cout << "  Ay simpleng identity — hindi formula\n\n";

    cout << "  PERO may emergent na pattern:\n";
    cout << "  Kapag |a-b| = 1:\n";
    cout << "  φ^a + φ^(a-1) = φ^(a+1)\n";
    cout << "  (dahil sa φ² = φ+1)\n\n";

    cout << "  Kapag |a-b| = 2:\n";
    cout << "  φ^a + φ^(a-2) = φ^(a-2) × (1 + φ²)\n";
    cout << "  = φ^(a-2) × (φ² + 1)\n";
    cout << "  = φ^(a-2) × (φ+2)\n\n";

    return 0;
}
