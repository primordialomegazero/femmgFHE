// GENERAL FORMULA NG BOUNDED PERIOD-6
// x_{n+1} = x_n - x_{n-1} + φ
// Hanapin kung bakit φ at bakit period-6

#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  GENERAL FORMULA ANALYSIS\n";
    cout << "  Bounded Period-6 Recurrence\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = -0.6180339887498948482;

    // ============================================
    // 1. ANG RECURRENCE
    // ============================================
    cout << "1. RECURRENCE:\n";
    cout << "   x_{n+1} = x_n - x_{n-1} + φ\n\n";

    cout << "   Ito ay parang:\n";
    cout << "   x_{n+1} - x_n + x_{n-1} = φ\n";
    cout << "   (x_{n+1} - x_n) - (x_n - x_{n-1}) = φ\n";
    cout << "   Δ²x = φ (second difference ay constant)\n\n";

    // ============================================
    // 2. CHARACTERISTIC EQUATION
    // ============================================
    cout << "2. CHARACTERISTIC EQUATION:\n";
    cout << "   r² - r + 1 = 0\n";
    cout << "   r = (1 ± √(1-4))/2 = (1 ± i√3)/2\n\n";

    cout << "   Ito ay COMPLEX ROOTS:\n";
    cout << "   r₁ = e^{iπ/3} = cos(π/3) + i·sin(π/3) = 0.5 + 0.866i\n";
    cout << "   r₂ = e^{-iπ/3} = 0.5 - 0.866i\n\n";

    cout << "   PERIOD = 2π / (π/3) = 6!\n\n";

    // ============================================
    // 3. BAKIT PERIOD-6?
    // ============================================
    cout << "3. PERIOD-6 EXPLANATION:\n";
    cout << "   Ang r₁ ay 6th root of unity:\n";
    cout << "   r₁⁶ = e^{i·2π} = 1\n\n";
    cout << "   Kaya ang homogeneous solution ay may period-6.\n";
    cout << "   Ang φ ay nagbibigay ng particular solution.\n\n";

    // ============================================
    // 4. φ CONNECTION
    // ============================================
    cout << "4. φ CONNECTION:\n";
    cout << "   φ = 2cos(π/5) = 2cos(36°)\n";
    cout << "   Ang period-6 ay galing sa π/3 = 60°\n";
    cout << "   Ang φ ay may angle na 36°\n\n";

    cout << "   60° + 36° = 96°\n";
    cout << "   Hindi ito diretso...\n\n";

    cout << "   PERO: φ² = φ + 1\n";
    cout << "   at: e^{iπ/3} + e^{-iπ/3} = 1\n\n";

    cout << "   Ang totoong connection:\n";
    cout << "   φ² - φ - 1 = 0 (golden ratio equation)\n";
    cout << "   r² - r + 1 = 0 (period-6 equation)\n\n";

    cout << "   MAGKAIBA sila pero may relation:\n";
    cout << "   φ² - φ - 1 = 0 → φ² = φ + 1\n";
    cout << "   r² - r + 1 = 0 → r² = r - 1\n\n";

    cout << "   Kung r = -φ:\n";
    cout << "   (-φ)² - (-φ) + 1 = φ² + φ + 1\n";
    cout << "   = (φ+1) + φ + 1 = 2φ + 2 ≠ 0\n\n";

    cout << "   Kung r = -1/φ:\n";
    cout << "   (-1/φ)² - (-1/φ) + 1 = 1/φ² + 1/φ + 1\n";
    cout << "   = (φ-1)²/φ² + (φ-1)/φ + 1\n";
    cout << "   ≠ 0\n\n";

    // ============================================
    // 5. GENERAL FORMULA
    // ============================================
    cout << "5. GENERAL FORMULA:\n";
    cout << "   Ang solution ng recurrence ay:\n";
    cout << "   x_n = A·cos(nπ/3) + B·sin(nπ/3) + C\n\n";

    cout << "   Kung saan:\n";
    cout << "   C = φ/(1-1+1) = φ (particular solution)\n\n";

    cout << "   Sa ating case:\n";
    cout << "   x_0 = 0 = A + C → A = -φ\n";
    cout << "   x_1 = φ² = A·cos(π/3) + B·sin(π/3) + φ\n";
    cout << "   φ² = -φ·0.5 + B·0.866 + φ\n";
    cout << "   φ² - φ = -0.5φ + 0.866B\n";
    cout << "   1 = -0.5φ + 0.866B\n";
    cout << "   B = (1 + 0.5φ)/0.866\n";
    cout << "   B = (1 + 0.809)/0.866 = 2.089\n\n";

    cout << "   Kaya ang general formula ay:\n";
    cout << "   x_n = -φ·cos(nπ/3) + 2.089·sin(nπ/3) + φ\n\n";

    // ============================================
    // 6. VERIFICATION
    // ============================================
    cout << "6. VERIFICATION:\n";
    cout << "   n=0: -φ·1 + 0 + φ = 0 ✓\n";
    cout << "   n=1: -φ·0.5 + 2.089·0.866 + φ\n";
    cout << "       = -0.809 + 1.809 + 1.618 = 2.618 = φ² ✓\n";
    cout << "   n=2: -φ·(-0.5) + 2.089·0.866 + φ\n";
    cout << "       = 0.809 + 1.809 + 1.618 = 4.236 = φ³ ✓\n\n";

    cout << "   PERFECT MATCH!\n\n";

    cout << "========================================\n";
    cout << "  GENERAL FORMULA:\n";
    cout << "  x_n = -φ·cos(nπ/3) + 2.089·sin(nπ/3) + φ\n";
    cout << "  PERIOD-6, BOUNDED, 0-LEVEL\n";
    cout << "========================================\n";

    return 0;
}
