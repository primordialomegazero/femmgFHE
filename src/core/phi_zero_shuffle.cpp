// ============================================
// φ-ZERO SHUFFLE — Zero bilang Anchor
// Ang zero slot ay fixed point sa shuffle
// Natural na anchor para sa permutation
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-ZERO SHUFFLE ===\n\n";
    cout << fixed << setprecision(12);

    int M = 32768;

    // ============================================
    // 1. Zero bilang fixed point
    // ============================================
    cout << "--- 1. Zero bilang fixed point ---\n\n";
    cout << "  slot(0) = 0 — fixed point\n";
    cout << "  Ang zero ay hindi gumagalaw sa shuffle\n\n";

    // Ang zero slot ay may special na property:
    // φ⁰ mod φ = 1, pero 0 mod φ = 0
    cout << "  0 mod φ = " << fmod(0.0, PHI) << "\n";
    cout << "  φ⁰ mod φ = " << fmod(pow(PHI, 0), PHI) << "\n\n";

    // ============================================
    // 2. Zero at non-zero sa shuffle
    // ============================================
    cout << "--- 2. Zero at non-zero ---\n\n";
    cout << "  Ang shuffle ay may dalawang bahagi:\n";
    cout << "  - Zero slot: fixed\n";
    cout << "  - Non-zero slots: nag-shuffle\n\n";

    // I-shuffle ang non-zero slots
    vector<int> non_zero_slots;
    for (int i = 1; i < M; i++) {
        int slot = (int)floor(i * M / (PHI * PHI)) % M;
        non_zero_slots.push_back(slot);
    }

    // Check kung may zero sa non-zero shuffle
    bool has_zero = false;
    for (int slot : non_zero_slots) {
        if (slot == 0) {
            has_zero = true;
            break;
        }
    }
    
    cout << "  May zero sa non-zero shuffle? "
         << (has_zero ? "✅ (collision sa anchor)" : "❌ (walang collision)") << "\n\n";

    // ============================================
    // 3. Zero bilang φ-identity
    // ============================================
    cout << "--- 3. Zero bilang φ-identity ---\n\n";
    cout << "  Ang zero ay may self-referential na φ-identity:\n";
    cout << "  0 = φ⁰ - φ⁰\n";
    cout << "  0 = φ⁻¹ - φ⁻¹\n";
    cout << "  0 = φ - φ\n\n";

    // Ang zero ay may natural na φ-gap
    cout << "  φ-gap na may zero:\n";
    cout << "  φ¹ - φ⁰ = " << pow(PHI, 1) - pow(PHI, 0) << " (hindi zero)\n";
    cout << "  φ⁰ - φ⁰ = " << pow(PHI, 0) - pow(PHI, 0) << " (zero!)\n\n";

    // ============================================
    // 4. Zero-anchored shuffle
    // ============================================
    cout << "--- 4. Zero-anchored shuffle ---\n\n";
    cout << "  Kung ang zero ay fixed anchor, ang shuffle\n";
    cout << "  ay may natural na structure\n\n";

    // Ang shuffle na may zero na naka-anchor
    cout << "  Slot 0: fixed (anchor)\n";
    cout << "  Slot 1: " << (int)floor(1.0 * M / (PHI * PHI)) % M << "\n";
    cout << "  Slot 2: " << (int)floor(2.0 * M / (PHI * PHI)) % M << "\n";
    cout << "  Slot 3: " << (int)floor(3.0 * M / (PHI * PHI)) % M << "\n";
    cout << "  Slot 4: " << (int)floor(4.0 * M / (PHI * PHI)) % M << "\n\n";

    // ============================================
    // 5. Zero bilang cryptographic anchor
    // ============================================
    cout << "--- 5. Zero bilang cryptographic anchor ---\n\n";
    cout << "  Ang zero ay maaaring gamitin bilang\n";
    cout << "  natural na anchor para sa encryption\n\n";
    cout << "  - Fixed point para sa decryption\n";
    cout << "  - Natural na reference para sa permutation\n";
    cout << "  - φ-identity para sa key generation\n\n";

    return 0;
}
