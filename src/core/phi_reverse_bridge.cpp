// ============================================
// φ-REVERSE BRIDGE
// Subok kung ang x = (x × φ) - x/φ ay kayang
// i-convert ang log space papuntang normal space
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
    cout << "  φ-REVERSE BRIDGE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // REVERSE BRIDGE: x = (x × φ) - x/φ
    // ============================================

    cout << "========================================\n";
    cout << "  REVERSE BRIDGE: x = (x × φ) - x/φ\n";
    cout << "========================================\n\n";

    cout << "  x | x × φ | x/φ | (x × φ) - x/φ | Match?\n";
    cout << "  --|-------|-----|---------------|--------\n";

    for (double x : {2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 35.0}) {
        double mul_phi = x * PHI;
        double div_phi = x / PHI;
        double recovered = mul_phi - div_phi;
        bool match = abs(recovered - x) < 0.001;
        
        cout << "  " << setw(3) << x << " | "
             << setw(6) << fixed << setprecision(4) << mul_phi << " | "
             << setw(5) << div_phi << " | "
             << setw(11) << recovered << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST: (5 × 7) + 3 = 38
    // ============================================

    cout << "\n========================================\n";
    cout << "  TEST: (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    // Step 1: 5 × 7 = 35 sa log space
    double log_prod = log(5.0) / LN_PHI + log(7.0) / LN_PHI;
    double prod = pow(PHI, log_prod);
    cout << "  Step 1: log_φ(5) + log_φ(7) = " << log_prod << "\n";
    cout << "  φ^" << log_prod << " = " << prod << " (expected: 35)\n\n";

    // Step 2: I-convert ang log_prod papuntang normal space
    // Subok: x = (x × φ) - x/φ
    double recovered = prod * PHI - prod / PHI;
    cout << "  Step 2: (35 × φ) - 35/φ = " << recovered << " (expected: 35)\n";
    cout << "  Match: " << (abs(recovered - 35.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // Step 3: + 3
    double result = recovered + 3.0;
    cout << "  Step 3: " << recovered << " + 3 = " << result << " (expected: 38)\n";
    cout << "  Match: " << (abs(result - 38.0) < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "  Level: 0 (walang EvalMult)\n\n";

    return 0;
}
