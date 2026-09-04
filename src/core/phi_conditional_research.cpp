// ============================================
// φ-CONDITIONAL RESEARCH
// Homomorphic comparison sa φ-log space
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-CONDITIONAL RESEARCH ===\n\n";

    // ============================================
    // 1. ANG EXPONENT k BILANG BRANCH SIGNAL
    // ============================================
    cout << "--- 1. EXPONENT k BILANG BRANCH SIGNAL ---\n\n";
    cout << "  F | k | r | F > φ^5?\n";
    cout << "  --|---|----|--------\n";
    
    for (double F : {2.0, 5.0, 13.0, 34.0, 89.0, 233.0, 610.0}) {
        double k = floor(log(F) / LN_PHI);
        double r = F / pow(PHI, k);
        bool greater = k > 5;
        
        cout << "  " << setw(5) << F << " | "
             << setw(2) << (int)k << " | "
             << setw(5) << fixed << setprecision(3) << r << " | "
             << (greater ? "YES" : " no") << "\n";
    }

    // ============================================
    // 2. ANG KEY: k AY INTEGER, COMPARISON AY SIMPLE
    // ============================================
    cout << "\n--- 2. k AY INTEGER ---\n\n";
    cout << "  Ang comparison F > φ^T ay katumbas ng k > T\n";
    cout << "  Ito ay INTEGER comparison — hindi kailangan\n";
    cout << "  ng homomorphic comparison\n\n";
    
    cout << "  T | k > T? (test k=5)\n";
    cout << "  --|------------------\n";
    for (int T : {3, 4, 5, 6, 7}) {
        bool result = 5 > T;
        cout << "  " << setw(2) << T << " | "
             << (result ? "YES" : " no") << "\n";
    }

    // ============================================
    // 3. ANG BRANCH DELTA
    // ============================================
    cout << "\n--- 3. BRANCH DELTA ---\n\n";
    cout << "  Kung k > T: delta = branch_A\n";
    cout << "  Kung k ≤ T: delta = branch_B\n\n";
    
    cout << "  Ito ay maaaring i-encode bilang:\n";
    cout << "  delta = step(k - T) × (branch_A - branch_B) + branch_B\n\n";
    cout << "  Kung saan step(x) = 1 kung x > 0, 0 kung x ≤ 0\n";
    cout << "  At ang step ay nasa Slot 0 (integer)\n\n";

    // ============================================
    // 4. ANG NATURAL NA STEP FUNCTION
    // ============================================
    cout << "--- 4. NATURAL NA STEP ---\n\n";
    cout << "  Ang step(k - T) ay maaaring i-express bilang:\n";
    cout << "  step(x) = (x + |x|) / (2x) para sa x ≠ 0\n";
    cout << "  step(0) = 0.5 (undefined sa totoong step)\n\n";
    
    cout << "  x | step(x)\n";
    cout << "  --|--------\n";
    for (int x : {-3, -2, -1, 0, 1, 2, 3}) {
        double step;
        if (x == 0) step = 0.5;
        else step = (x + abs(x)) / (2.0 * x);
        
        cout << "  " << setw(2) << x << " | "
             << setw(5) << fixed << setprecision(2) << step << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang conditional sa φ-log space ay:\n";
    cout << "  - Integer comparison sa Slot 0 (k)\n";
    cout << "  - Step function na naka-encode sa φ-structure\n";
    cout << "  - Branch delta bilang EvalAdd\n\n";

    return 0;
}
