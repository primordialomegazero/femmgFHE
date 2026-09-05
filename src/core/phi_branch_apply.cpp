// ============================================
// φ-BRANCH APPLY
// I-apply ang branch delta sa pure FHE
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "=== φ-BRANCH APPLY ===\n\n";

    // ============================================
    // 1. ANG SLOT 0 DIFFERENCE BILANG SIGNAL
    // ============================================
    cout << "--- 1. SLOT 0 DIFFERENCE ---\n\n";
    cout << "  Slot 0 = log_φ(F)\n";
    cout << "  Threshold = 5\n";
    cout << "  δ_log = Slot 0 - 5\n\n";
    
    cout << "  F | Slot 0 | δ_log | Branch | δ_apply\n";
    cout << "  --|--------|-------|--------|---------\n";
    
    for (double F : {3.0, 5.0, 8.0, 11.0, 12.0, 15.0, 20.0}) {
        double slot0 = log(F) / LN_PHI;
        double delta_log = slot0 - 5.0;
        string branch = (delta_log > 0) ? "A" : "B";
        double delta_apply = (delta_log > 0) ? 1.0 : -1.0;
        
        cout << "  " << setw(5) << F << " | "
             << setw(7) << fixed << setprecision(3) << slot0 << " | "
             << setw(7) << delta_log << " | "
             << setw(4) << branch << " | "
             << setw(5) << delta_apply << "\n";
    }

    // ============================================
    // 2. ANG KEY: δ_log AY ENCRYPTED SA SLOT 0
    // ============================================
    cout << "\n--- 2. δ_log AY ENCRYPTED ---\n\n";
    cout << "  Ang δ_log ay nasa Slot 0 — naka-encrypt\n";
    cout << "  Hindi natin kailangang i-decrypt\n";
    cout << "  Ang sign ay automatic sa φ-structure\n\n";

    // ============================================
    // 3. ANG PAG-APPLY NANG WALANG EVALMULT
    // ============================================
    cout << "--- 3. PAG-APPLY NANG WALANG EVALMULT ---\n\n";
    cout << "  Ang δ_apply = ±1 ay kailangang i-apply\n";
    cout << "  bilang delta sa log space\n\n";
    cout << "  Ito ay maaaring gawin sa pamamagitan ng:\n";
    cout << "  EvalAdd ng φ-based na threshold mask\n\n";
    
    cout << "  Ang threshold mask ay:\n";
    cout << "  mask = φ^(δ_log) / (φ^(δ_log) + 1)\n\n";
    
    cout << "  δ_log | φ^(δ_log) | mask | δ_apply\n";
    cout << "  -------|-----------|------|---------\n";
    
    for (double delta_log : {-3.0, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 3.0}) {
        double phi_delta = pow(PHI, delta_log);
        double mask = phi_delta / (phi_delta + 1.0);
        double delta_apply = 2.0 * mask - 1.0;
        
        cout << "  " << setw(7) << fixed << setprecision(1) << delta_log << " | "
             << setw(8) << phi_delta << " | "
             << setw(6) << mask << " | "
             << setw(7) << delta_apply << "\n";
    }

    cout << "\n=== KEY ===\n";
    cout << "  Ang mask = φ^(δ_log)/(φ^(δ_log)+1) ay may\n";
    cout << "  natural na branch behavior\n";
    cout << "  δ_apply = 2×mask - 1 ay nasa [-1, 1]\n";
    cout << "  Ito ay maaaring i-encode bilang EvalAdd\n\n";

    return 0;
}
