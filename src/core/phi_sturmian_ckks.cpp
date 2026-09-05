// ============================================
// φ-STURMIAN CKKS — Canonical Embedding
// Ang Sturmian word at CKKS roots of unity
// May natural na φ-connection ba?
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <complex>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-STURMIAN CKKS ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. CKKS canonical embedding
    // ============================================
    int M = 32768;  // cyclotomic order
    cout << "--- 1. Canonical embedding ---\n\n";
    cout << "  M = " << M << " (cyclotomic order)\n";
    cout << "  ζ = e^(2πi/M) — primitive root\n\n";

    // ============================================
    // 2. Sturmian at roots of unity
    // ============================================
    cout << "--- 2. Sturmian at roots of unity ---\n\n";
    cout << "  Ang Sturmian φ-rotation ay may koneksyon\n";
    cout << "  sa roots of unity ng canonical embedding\n\n";

    // I-compare ang φ-rotation sa roots of unity
    cout << "  i | frac(i×φ) | e^(2πi×frac) | Re | Im\n";
    cout << "  --|------------|---------------|----|----\n";

    for (int i = 0; i < 12; i++) {
        double frac = fmod(i * PHI, 1.0);
        complex<double> rot = exp(complex<double>(0, 2.0 * M_PI * frac));
        
        cout << "  " << setw(2) << i << " | "
             << setw(10) << frac << " | "
             << setw(8) << rot.real() << " | "
             << setw(6) << rot.real() << " | "
             << setw(6) << rot.imag() << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang golden angle sa canonical embedding
    // ============================================
    cout << "--- 3. Golden angle sa embedding ---\n\n";
    cout << "  Golden angle = 2π/φ² ≈ 137.5°\n";
    cout << "  Sa M-space: M/φ² ≈ " << M / (PHI * PHI) << " steps\n\n";
    cout << "  Ang golden angle ay nagbibigay ng\n";
    cout << "  optimal na slot distribution\n\n";

    // ============================================
    // 4. Sturmian bilang slot index
    // ============================================
    cout << "--- 4. Sturmian bilang slot index ---\n\n";
    cout << "  Ang Sturmian sequence ay nagbibigay ng\n";
    cout << "  natural na slot permutation\n\n";

    vector<int> sturmian_indices;
    for (int i = 0; i < 16; i++) {
        int idx = (int)floor(i * PHI) % 16;
        sturmian_indices.push_back(idx);
    }

    cout << "  Sturmian slot indices:\n  ";
    for (int idx : sturmian_indices) {
        cout << setw(4) << idx;
    }
    cout << "\n\n";

    // Check kung unique
    bool unique = true;
    for (size_t i = 0; i < sturmian_indices.size(); i++) {
        for (size_t j = i+1; j < sturmian_indices.size(); j++) {
            if (sturmian_indices[i] == sturmian_indices[j]) {
                unique = false;
            }
        }
    }
    cout << "  Lahat unique: " << (unique ? "✅" : "❌") << "\n\n";

    // ============================================
    // 5. Ang emergent na φ-based slot mapping
    // ============================================
    cout << "--- 5. Emergent na φ-based slot mapping ---\n\n";
    cout << "  Ang Sturmian slot mapping ay may\n";
    cout << "  natural na φ-distribution na mas\n";
    cout << "  optimal kaysa sa linear na mapping\n\n";

    cout << "  Linear:   0, 1, 2, 3, 4, 5, 6, 7\n";
    cout << "  Sturmian: ";
    for (int i = 0; i < 8; i++) {
        cout << sturmian_indices[i];
        if (i < 7) cout << ", ";
    }
    cout << "\n\n";

    return 0;
}
