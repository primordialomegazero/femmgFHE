// ============================================
// φ-POWER MULT-DIV
// Subok kung multiplication at division
// ay pwedeng gawin sa φ-power space
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
    cout << "  φ-POWER MULT-DIV\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // MULTIPLICATION SA φ-POWER SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  MULTIPLICATION SA φ-POWER SPACE\n";
    cout << "========================================\n\n";

    cout << "  φ^m × φ^n = φ^(m+n)\n";
    cout << "  Sa power space: m + n\n\n";

    cout << "  m | n | φ^m × φ^n | φ^(m+n) | Match?\n";
    cout << "  --|---|-----------|---------|--------\n";

    for (int m : {1, 2, 3, 4, 5}) {
        for (int n : {1, 2, 3, 4, 5}) {
            double phi_mn = pow(PHI, m) * pow(PHI, n);
            double phi_sum = pow(PHI, m + n);
            bool match = abs(phi_mn - phi_sum) < 0.001;
            
            cout << "  " << m << " | " << n
                 << " | " << fixed << setprecision(4) << phi_mn
                 << " | " << phi_sum
                 << " | " << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // DIVISION SA φ-POWER SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  DIVISION SA φ-POWER SPACE\n";
    cout << "========================================\n\n";

    cout << "  φ^m ÷ φ^n = φ^(m-n)\n";
    cout << "  Sa power space: m - n\n\n";

    cout << "  m | n | φ^m ÷ φ^n | φ^(m-n) | Match?\n";
    cout << "  --|---|-----------|---------|--------\n";

    for (int m : {2, 3, 4, 5}) {
        for (int n : {1, 2, 3}) {
            double phi_div = pow(PHI, m) / pow(PHI, n);
            double phi_diff = pow(PHI, m - n);
            bool match = abs(phi_div - phi_diff) < 0.001;
            
            cout << "  " << m << " | " << n
                 << " | " << fixed << setprecision(4) << phi_div
                 << " | " << phi_diff
                 << " | " << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // ALL OPERATIONS SA φ-POWER SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  ALL OPERATIONS SA φ-POWER SPACE\n";
    cout << "========================================\n\n";

    cout << "  Addition:   φ^m + φ^(m-1) = φ^(m+1) → (m) + (m-1) = m+1\n";
    cout << "  Subtraction: φ^(m+1) - φ^m = φ^(m-1) → (m+1) - m = m-1\n";
    cout << "  Multiplication: φ^m × φ^n = φ^(m+n) → m + n\n";
    cout << "  Division:  φ^m ÷ φ^n = φ^(m-n) → m - n\n\n";

    cout << "  LAHAT AY ADDITION AT SUBTRACTION SA INDEX!\n";
    cout << "  Walang multiplication sa φ-value!\n\n";

    // ============================================
    // VERIFICATION SA INDEX SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  VERIFICATION SA INDEX SPACE\n";
    cout << "========================================\n\n";

    cout << "  Addition: m + (m-1) = 2m-1\n";
    cout << "  Pero dapat: m+1\n";
    cout << "  PROBLEMA!\n\n";

    for (int m : {1, 2, 3, 4, 5}) {
        double add_index = m + (m - 1);
        double expected = m + 1;
        cout << "  m=" << m << ": index add = " << add_index 
             << ", expected = " << expected 
             << (add_index == expected ? " ✅" : " ❌") << "\n";
    }

    return 0;
}
