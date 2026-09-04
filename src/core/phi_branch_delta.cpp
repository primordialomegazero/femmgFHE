// ============================================
// φ-BRANCH DELTA RESEARCH
// Hanapin ang branch delta na walang EvalMult
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-BRANCH DELTA RESEARCH ===\n\n";

    // ============================================
    // 1. ANG F/φ^5 BILANG BRANCH SIGNAL
    // ============================================
    cout << "--- 1. F/φ^5 BILANG SIGNAL ---\n\n";
    cout << "  F | F/φ^5 | φ^(log-5) | Branch\n";
    cout << "  --|-------|-----------|-------\n";
    
    for (double F : {3.0, 5.0, 8.0, 11.0, 12.0, 15.0, 20.0, 30.0, 50.0, 100.0}) {
        double log_F = log(F) / LN_PHI;
        double ratio = F / pow(PHI, 5);
        double phi_diff = pow(PHI, log_F - 5);
        
        cout << "  " << setw(5) << F << " | "
             << setw(7) << fixed << setprecision(3) << ratio << " | "
             << setw(7) << phi_diff << " | "
             << (phi_diff > 1 ? "A" : "B") << "\n";
    }

    // ============================================
    // 2. ANG DELTA BILANG FUNCTION NG SIGNAL
    // ============================================
    cout << "\n--- 2. DELTA BILANG FUNCTION ---\n\n";
    cout << "  Kung F > φ^5: delta = +1 (×φ)\n";
    cout << "  Kung F ≤ φ^5: delta = -1 (÷φ)\n\n";
    
    cout << "  F | F/φ^5 | δ (exact) | δ (φ-based) | Match?\n";
    cout << "  --|-------|-----------|--------------|-------\n";
    
    for (double F : {3.0, 5.0, 8.0, 11.0, 12.0, 15.0, 20.0}) {
        double ratio = F / pow(PHI, 5);
        double delta_exact = (ratio > 1.0) ? 1.0 : -1.0;
        // φ-based: delta = (ratio - 1/ratio) / |ratio - 1/ratio|
        double inv_ratio = 1.0 / ratio;
        double numerator = ratio - inv_ratio;
        double denominator = abs(numerator);
        double delta_phi = numerator / denominator;
        
        cout << "  " << setw(5) << F << " | "
             << setw(7) << fixed << setprecision(3) << ratio << " | "
             << setw(5) << delta_exact << " | "
             << setw(8) << delta_phi << " | "
             << (abs(delta_exact - delta_phi) < 0.01 ? "✅" : "❌") << "\n";
    }

    // ============================================
    // 3. ANG KEY: NATURAL NA SIGN EXTRACTION
    // ============================================
    cout << "\n--- 3. NATURAL NA SIGN EXTRACTION ---\n\n";
    cout << "  Ang (ratio - 1/ratio) ay may sign na\n";
    cout << "  tumutugma sa branch — walang comparison\n\n";
    
    cout << "  ratio | ratio - 1/ratio | sign\n";
    cout << "  -------|-----------------|------\n";
    
    for (double ratio : {0.5, 0.8, 0.99, 1.01, 1.5, 2.0}) {
        double diff = ratio - 1.0/ratio;
        char sign = (diff > 0) ? '+' : '-';
        
        cout << "  " << setw(7) << fixed << setprecision(3) << ratio << " | "
             << setw(9) << diff << " | "
             << sign << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang (F/φ^5 - φ^5/F) ay may natural na sign\n";
    cout << "  na tumutugma sa branch — walang comparison\n";
    cout << "  Ang pag-apply ng delta ay nangangailangan pa\n";
    cout << "  ng EvalMult, ngunit ang sign extraction ay free\n\n";

    return 0;
}
