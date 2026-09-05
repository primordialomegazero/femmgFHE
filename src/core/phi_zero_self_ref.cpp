// ============================================
// φ-ZERO SELF-REF — Zero State Pattern
// next=0: val ∈ {0, 3, 7}
// Hanapin ang self-referential na pattern
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <bitset>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-ZERO SELF-REF ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Ang zero states
    // ============================================
    cout << "--- 1. Zero states ---\n\n";
    cout << "  next=0: val ∈ {0, 3, 7}\n\n";

    cout << "  val | Binary | φ-representation\n";
    cout << "  ----|--------|-----------------\n";
    
    for (int val : {0, 3, 7}) {
        cout << "  " << setw(3) << val << " | "
             << setw(6) << bitset<3>(val) << " | ";
        
        // φ-based na representation
        double remaining = val;
        for (int p = 3; p >= -2; p--) {
            double phi_p = pow(PHI, p);
            if (remaining >= phi_p - 1e-6) {
                cout << "φ^" << p << " ";
                remaining -= phi_p;
            }
        }
        cout << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Ang self-referential na pattern
    // ============================================
    cout << "--- 2. Self-referential na pattern ---\n\n";
    cout << "  Ang zero states ay may φ-based na self-reference:\n";
    cout << "  0 → 000 → walang φ-power\n";
    cout << "  3 → 011 → may φ¹ at φ⁰\n";
    cout << "  7 → 111 → may φ², φ¹, φ⁰\n\n";

    // ============================================
    // 3. Ang threshold na may φ-mod
    // ============================================
    cout << "--- 3. Threshold na may φ-mod ---\n\n";
    cout << "  val | val mod φ | Next\n";
    cout << "  ----|-----------|------\n";
    
    for (int val = 0; val < 8; val++) {
        double mod_phi = fmod(val, PHI);
        int next = rule110[val];
        
        cout << "  " << setw(3) << val << " | "
             << setw(8) << mod_phi << " |  "
             << next << "\n";
    }
    cout << "\n";

    // ============================================
    // 4. Ang natural na zero detection
    // ============================================
    cout << "--- 4. Natural na zero detection ---\n\n";
    cout << "  Ang φ^n mod φ ay may natural na zero:\n";
    cout << "  φ^even → 1, φ^odd → φ⁻¹\n";
    cout << "  Ang zero ay automatic sa parity\n\n";

    cout << "  val | parity | φ^val mod φ | Next\n";
    cout << "  ----|--------|-------------|------\n";
    for (int val = 0; val < 8; val++) {
        double phi_val = pow(PHI, val);
        double mod_phi = fmod(phi_val, PHI);
        int parity = val % 2;
        int next = rule110[val];
        
        cout << "  " << setw(3) << val << " | "
             << setw(6) << parity << " | "
             << setw(10) << mod_phi << " |  "
             << next << "\n";
    }
    cout << "\n";

    // ============================================
    // 5. Ang φ-based na transition formula
    // ============================================
    cout << "--- 5. φ-based na transition formula ---\n\n";
    cout << "  next = 1 kung val ∈ {1, 2, 4, 5, 6}\n";
    cout << "  next = 0 kung val ∈ {0, 3, 7}\n\n";
    
    cout << "  Ang pattern:\n";
    cout << "  val mod 4 = 0 at val ≠ 0 → next=1 (val=4)\n";
    cout << "  val mod 3 = 0 at val ≠ 0 → next=0 (val=3,6) o next=1 (val=6)\n\n";
    
    cout << "  val | val mod 4 | Next\n";
    cout << "  ----|-----------|------\n";
    for (int val = 0; val < 8; val++) {
        int mod4 = val % 4;
        int next = rule110[val];
        
        cout << "  " << setw(3) << val << " | "
             << setw(9) << mod4 << " |  "
             << next << "\n";
    }
    cout << "\n";

    return 0;
}
