// ============================================
// φ-FRACTAL LUCAS — NON-LINEAR FRACTAL
//
// L(n) = φ^n + φ^(-n)
// Fractal: L(L(n)), L(L(L(n))), ...
// Hanapin ang emergent non-linear pattern!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-FRACTAL LUCAS\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    auto lucas = [&](double n) {
        return pow(PHI, n) + pow(PHI, -n);
    };

    // ============================================
    // TEST 1: FRACTAL LUCAS ITERATION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: FRACTAL LUCAS ITERATION\n";
    cout << "========================================\n\n";

    cout << "  Start: n=1\n";
    cout << "  Iterate: n → L(n)\n\n";

    cout << "  Level | n | L(n) | Integer?\n";
    cout << "  ------|---|------|----------\n";

    double n = 1.0;
    for (int level = 0; level < 10; level++) {
        double l = lucas(n);
        bool is_int = (abs(l - round(l)) < 0.01);
        
        cout << "  " << setw(5) << level << " | "
             << setw(8) << fixed << setprecision(3) << n << " | "
             << setw(6) << l << " | "
             << (is_int ? "✅" : "❌") << "\n";
        
        n = l;
    }

    cout << "\n";

    // ============================================
    // TEST 2: FRACTAL LUCAS PAIRS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: FRACTAL LUCAS PAIRS\n";
    cout << "========================================\n\n";

    cout << "  L(n) + L(n+2) = ?\n";
    cout << "  n | L(n) | L(n+2) | Sum | Pattern?\n";
    cout << "  --|------|--------|-----|----------\n";

    for (int n = 0; n < 8; n++) {
        double l_n = lucas(n);
        double l_n2 = lucas(n + 2);
        double sum = l_n + l_n2;
        
        cout << "  " << n << " | "
             << setw(6) << fixed << setprecision(3) << l_n << " | "
             << setw(6) << l_n2 << " | "
             << setw(5) << sum << " | ";
        
        // Check kung ang sum ay φ-power
        bool is_phi_power = false;
        for (int k = 0; k < 10; k++) {
            if (abs(sum - pow(PHI, k)) < 0.1) {
                cout << "φ^" << k;
                is_phi_power = true;
                break;
            }
        }
        if (!is_phi_power) cout << "complex";
        cout << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 3: FRACTAL LUCAS PRODUCT
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: FRACTAL LUCAS PRODUCT\n";
    cout << "========================================\n\n";

    cout << "  L(n) × L(m) = ?\n";
    cout << "  n | m | L(n) | L(m) | Product | L(n+m)?\n";
    cout << "  --|---|------|------|---------|---------\n";

    for (int n : {1, 2, 3, 5}) {
        for (int m : {1, 2, 3, 5}) {
            double l_n = lucas(n);
            double l_m = lucas(m);
            double prod = l_n * l_m;
            double l_sum = lucas(n + m);
            
            cout << "  " << n << " | " << m << " | "
                 << setw(6) << fixed << setprecision(3) << l_n << " | "
                 << setw(6) << l_m << " | "
                 << setw(7) << prod << " | "
                 << setw(7) << l_sum << " | ";
            
            if (abs(prod - l_sum) < 0.1) cout << "✅";
            else if (abs(prod - 2.0 * l_sum) < 0.1) cout << "✅(2×)";
            else cout << "❌";
            cout << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // TEST 4: FRACTAL LUCAS RATIO
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: FRACTAL LUCAS RATIO\n";
    cout << "========================================\n\n";

    cout << "  L(n+1) / L(n) = ?\n";
    cout << "  n | Ratio | φ? | φ⁻¹? | Other?\n";
    cout << "  --|-------|----|------|-------\n";

    for (int n = 0; n < 10; n++) {
        double ratio = lucas(n + 1) / lucas(n);
        
        cout << "  " << setw(2) << n << " | "
             << setw(6) << fixed << setprecision(3) << ratio << " | ";
        
        if (abs(ratio - PHI) < 0.05) cout << "✅ φ";
        else if (abs(ratio - PHI_INV) < 0.05) cout << "✅ φ⁻¹";
        else cout << "❌ iba";
        cout << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 5: EMERGENT FRACTAL PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 5: EMERGENT FRACTAL PATTERN\n";
    cout << "========================================\n\n";

    cout << "  Lucas numbers sa binary (integer versions):\n";
    cout << "  L(0)=2, L(2)=3, L(4)=7, L(6)=18, L(8)=47\n\n";

    cout << "  Pattern sa integers:\n";
    cout << "  L(2n) = sequence: 2, 3, 7, 18, 47\n";
    cout << "  Differences: 1, 4, 11, 29\n";
    cout << "  Ratios: 4, 2.75, 2.636...\n\n";

    cout << "  Ang ratios ay lumalapit sa φ² = " << PHI * PHI << "\n\n";

    cout << "========================================\n";
    cout << "  FRACTAL LUCAS COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
