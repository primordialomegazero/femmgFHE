// ============================================
// φ-ERGODIC — Natural na Pseudo-Random
// Ang φ-rotation sa full M-space ay ergodic
// Deterministic pero parang random
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-ERGODIC ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Ergodic property sa M-space
    // ============================================
    cout << "--- 1. Ergodic sa M=32768 ---\n\n";
    cout << "  Ang φ-rotation ay dumadalaw sa lahat\n";
    cout << "  ng points sa complex circle\n\n";

    int M = 32768;
    double golden_step = M / (PHI * PHI);
    cout << "  Golden step: " << golden_step << "\n";
    cout << "  Fractional: " << golden_step - floor(golden_step) << "\n\n";

    // I-verify ang ergodic property
    cout << "  i | slot | frac(i×φ)\n";
    cout << "  --|------|----------\n";
    for (int i = 0; i < 10; i++) {
        double frac = fmod(i * PHI, 1.0);
        int slot = (int)(frac * M);
        cout << "  " << setw(2) << i << " | "
             << setw(5) << slot << " | "
             << setw(10) << frac << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Pseudo-random analysis
    // ============================================
    cout << "--- 2. Pseudo-random analysis ---\n\n";
    cout << "  Ang φ-rotation ay parang random pero\n";
    cout << "  deterministic — reproducible\n\n";

    // Test: distribution ng 100 values
    vector<int> buckets(10, 0);
    for (int i = 0; i < 1000; i++) {
        double frac = fmod(i * PHI, 1.0);
        int bucket = (int)(frac * 10);
        buckets[bucket]++;
    }

    cout << "  Distribution (1000 values sa 10 buckets):\n";
    cout << "  Bucket | Count | %\n";
    cout << "  -------|-------|----\n";
    for (int i = 0; i < 10; i++) {
        cout << "    " << setw(2) << i << "   | "
             << setw(5) << buckets[i] << " | "
             << setw(5) << (buckets[i] / 10.0) << "%\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ bilang pseudo-random generator
    // ============================================
    cout << "--- 3. φ bilang PRNG ---\n\n";
    cout << "  Ang φ-rotation ay may natural na\n";
    cout << "  pseudo-random na properties\n\n";

    // I-generate ang pseudo-random na sequence
    cout << "  Sequence (first 20):\n  ";
    for (int i = 0; i < 20; i++) {
        double frac = fmod(i * PHI, 1.0);
        cout << setw(6) << frac;
        if (i % 5 == 4) cout << "\n  ";
    }
    cout << "\n\n";

    // ============================================
    // 4. Ang ergodic na encryption
    // ============================================
    cout << "--- 4. Ergodic na encryption ---\n\n";
    cout << "  Ang φ-rotation ay maaaring gamitin para sa\n";
    cout << "  deterministic na encryption na parang random\n\n";

    // Simple XOR-like na φ-encryption
    cout << "  Original values: 1, 2, 3, 4, 5\n";
    cout << "  φ-keys: 0.618, 0.236, 0.854, 0.472, 0.090\n";
    cout << "  Encrypted: ";

    for (int i = 0; i < 5; i++) {
        double key = fmod(i * PHI, 1.0);
        double val = i + 1.0;
        double encrypted = fmod(val + key, 1.0);
        cout << setw(6) << encrypted;
    }
    cout << "\n\n";

    return 0;
}
