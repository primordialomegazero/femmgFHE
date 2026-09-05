// ============================================
// φ-ZERO CRYPTO — Zero-Anchored Encryption
// Ang zero bilang natural na cryptographic
// anchor sa φ-shuffle
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-ZERO CRYPTO ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Zero bilang encryption key
    // ============================================
    cout << "--- 1. Zero bilang encryption key ---\n\n";
    cout << "  Ang zero ay may φ-identity:\n";
    cout << "  0 = φ - φ = φ⁻¹ - φ⁻¹\n";
    cout << "  Ito ay natural na symmetric key\n\n";

    // Ang zero key ay may dual nature:
    // - Additive identity: a + 0 = a
    // - Multiplicative zero: a × 0 = 0
    cout << "  Additive zero: φ + 0 = " << PHI + 0.0 << "\n";
    cout << "  Multiplicative zero: φ × 0 = " << PHI * 0.0 << "\n\n";

    // ============================================
    // 2. Zero-anchored permutation
    // ============================================
    cout << "--- 2. Zero-anchored permutation ---\n\n";
    
    int M = 32768;
    
    // Ang permutation na may zero na fixed
    vector<int> perm(M, 0);
    perm[0] = 0;  // zero anchor
    
    for (int i = 1; i < M; i++) {
        perm[i] = (int)floor(i * M / (PHI * PHI)) % M;
    }
    
    cout << "  Permutation na may zero anchor:\n";
    cout << "    perm(0) = 0 (fixed)\n";
    cout << "    perm(1) = " << perm[1] << "\n";
    cout << "    perm(2) = " << perm[2] << "\n";
    cout << "    perm(3) = " << perm[3] << "\n";
    cout << "    perm(4) = " << perm[4] << "\n\n";

    // ============================================
    // 3. Zero bilang φ-reference
    // ============================================
    cout << "--- 3. Zero bilang φ-reference ---\n\n";
    cout << "  Ang zero ay may natural na φ-reference:\n";
    cout << "  φ⁰ - 1 = 0\n";
    cout << "  φ¹ - φ = 0\n\n";

    cout << "  φ⁰ - 1 = " << pow(PHI, 0) - 1.0 << "\n";
    cout << "  φ¹ - φ = " << pow(PHI, 1) - PHI << "\n";
    cout << "  φ² - φ - 1 = " << pow(PHI, 2) - PHI - 1.0 << "\n\n";

    // ============================================
    // 4. Zero-anchored na φ-encryption
    // ============================================
    cout << "--- 4. Zero-anchored na φ-encryption ---\n\n";
    cout << "  Ang encryption ay may φ-based na zero key\n\n";

    // Simple na φ-based na encryption
    string message = "GOLDEN";
    cout << "  Message: " << message << "\n";
    
    cout << "  Encrypted: ";
    for (char c : message) {
        int val = c - 'A';
        double encrypted = fmod(val * PHI, 26.0);
        cout << setw(6) << encrypted;
    }
    cout << "\n\n";

    // ============================================
    // 5. Ang φ-key na may zero
    // ============================================
    cout << "--- 5. φ-key na may zero ---\n\n";
    cout << "  Ang φ-key ay may natural na zero:\n";
    cout << "  key = 0 → φ⁰ = 1 (identity)\n";
    cout << "  key = 1 → φ¹ = φ (rotation)\n";
    cout << "  key = 2 → φ² = φ+1 (growth)\n\n";

    cout << "  key | φ^key | φ^key mod φ\n";
    cout << "  ----|-------|-------------\n";
    for (int key = 0; key <= 5; key++) {
        cout << "  " << setw(3) << key << " | "
             << setw(6) << pow(PHI, key) << " | "
             << setw(8) << fmod(pow(PHI, key), PHI) << "\n";
    }
    cout << "\n";

    return 0;
}
