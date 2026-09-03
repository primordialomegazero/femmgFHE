// ============================================
// φ-VERIFY 10K — TAMA BA ANG RESULT?
// I-compare ang encrypted result sa plaintext simulation
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-VERIFY 10K\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    // ============================================
    // PLAINTEXT SIMULATION
    // ============================================

    double expected = 1.0;
    
    for (int i = 0; i < 10000; i++) {
        switch (i % 4) {
            case 0: expected *= 2.0; break;
            case 1: expected /= 2.0; break;
            case 2: expected += 3.0; break;
            case 3: expected -= 1.0; break;
        }
    }

    double expected_mod_phi = fmod(expected, PHI);

    cout << "  Plaintext simulation:\n";
    cout << "  Expected raw: " << expected << "\n";
    cout << "  Expected mod φ: " << expected_mod_phi << "\n\n";

    // ============================================
    // ENCRYPTED RESULT (mula sa test)
    // ============================================

    double encrypted_raw = 955.915;
    double encrypted_mod_phi = fmod(encrypted_raw, PHI);

    cout << "  Encrypted result:\n";
    cout << "  Encrypted raw: " << encrypted_raw << "\n";
    cout << "  Encrypted mod φ: " << encrypted_mod_phi << "\n\n";

    // ============================================
    // COMPARISON
    // ============================================

    cout << "  Comparison:\n";
    cout << "  Expected mod φ: " << expected_mod_phi << "\n";
    cout << "  Encrypted mod φ: " << encrypted_mod_phi << "\n";
    cout << "  Match: " << (abs(expected_mod_phi - encrypted_mod_phi) < 0.01 ? "✅" : "❌") << "\n\n";

    return 0;
}
