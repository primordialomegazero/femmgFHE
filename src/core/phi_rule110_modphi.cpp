// ============================================
// φ-RULE 110 MODPHI — φ-mod Threshold
// next batay sa val mod φ — natural na threshold
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 MODPHI ===\n\n";
    cout << fixed << setprecision(15);

    cout << "  val | val mod φ | Next\n";
    cout << "  ----|-----------|------\n";
    for (int val = 0; val < 8; val++) {
        double mod_phi = fmod(val, PHI);
        int next = rule110[val];
        cout << "  " << setw(3) << val << " | "
             << setw(10) << mod_phi << " |  "
             << next << "\n";
    }
    cout << "\n";

    // Ang pattern ng mod φ:
    // next=0: mod ∈ {0, 1.382, 0.528}
    // next=1: mod ∈ {1, 0.382, 0.764, 0.146, 1.146}
    
    cout << "  next=0: mod φ ∈ {0.000, 1.382, 0.528}\n";
    cout << "  next=1: mod φ ∈ {1.000, 0.382, 0.764, 0.146, 1.146}\n\n";

    // Ang φ-threshold na naghihiwalay:
    cout << "  φ-threshold analysis:\n";
    cout << "  next=0 ay may mod φ na:\n";
    cout << "  - 0.000 (exact zero)\n";
    cout << "  - 0.528 (malapit sa φ⁻¹)\n";
    cout << "  - 1.382 (malapit sa φ)\n\n";

    // Subukan: next = 0 kung (val mod φ) mod φ⁻¹ == 0
    cout << "  Test: next = 0 kung (val mod φ) mod φ⁻¹ ≈ 0\n\n";
    cout << "  val | mod φ | mod φ⁻¹ | Next\n";
    cout << "  ----|-------|---------|------\n";
    for (int val = 0; val < 8; val++) {
        double mod_phi = fmod(val, PHI);
        double mod_phi_inv = fmod(mod_phi, PHI - 1.0);
        int next = rule110[val];
        
        cout << "  " << setw(3) << val << " | "
             << setw(6) << mod_phi << " | "
             << setw(8) << mod_phi_inv << " |  "
             << next << "\n";
    }
    cout << "\n";

    return 0;
}
