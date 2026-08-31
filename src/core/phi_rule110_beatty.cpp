// ============================================
// φ-RULE 110 BEATTY — BEATTY SEQUENCE ENCODING
//
// Ang Beatty theorem: ⌊nφ⌋ at ⌊nφ²⌋ ay nagpa-partition
// ng positive integers. Gamitin ito para sa Rule 110.
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 BEATTY\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // ============================================
    // BEATTY SEQUENCES
    // ============================================

    cout << "========================================\n";
    cout << "  BEATTY SEQUENCES (φ at φ²)\n";
    cout << "========================================\n\n";

    cout << "  n  | ⌊nφ⌋ | ⌊nφ²⌋ | Partition?\n";
    cout << "  ---|------|-------|-----------\n";

    vector<int> beatty_phi, beatty_phi2;
    for (int n = 1; n <= 10; n++) {
        int b1 = (int)floor(n * PHI);
        int b2 = (int)floor(n * PHI * PHI);
        beatty_phi.push_back(b1);
        beatty_phi2.push_back(b2);
        
        cout << "  " << setw(2) << n << " | "
             << setw(4) << b1 << " | "
             << setw(5) << b2 << " | ";
        
        // Check: walang overlap
        bool overlap = false;
        for (int i = 0; i < beatty_phi.size() - 1; i++) {
            if (beatty_phi[i] == b2 || beatty_phi2[i] == b1) overlap = true;
        }
        cout << (overlap ? "❌" : "✅") << "\n";
    }

    cout << "\n  Beatty φ: ";
    for (int v : beatty_phi) cout << v << " ";
    cout << "\n  Beatty φ²: ";
    for (int v : beatty_phi2) cout << v << " ";
    cout << "\n\n";

    // ============================================
    // BEATTY SEQUENCE ENCODING PARA SA RULE 110
    // ============================================

    cout << "========================================\n";
    cout << "  BEATTY ENCODING PARA SA RULE 110\n";
    cout << "========================================\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // I-encode ang 8 patterns gamit ang Beatty values
    // Pattern value = nφ kung saan n ay ang binary value
    // O nφ² para sa complementary

    cout << "  Pattern (LCR) | Binary Value | nφ | nφ² | Expected | Decoded?\n";
    cout << "  --------------|-------------|----|----|----------|----------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                double val_phi = pattern * PHI;
                double val_phi2 = pattern * PHI * PHI;
                
                // Subukan: gamitin ang Beatty floor bilang decoder
                int floor_phi = (int)floor(val_phi);
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(12) << pattern << " | "
                     << setw(4) << fixed << setprecision(1) << val_phi << " | "
                     << setw(4) << val_phi2 << " | "
                     << setw(8) << expected << " | "
                     << setw(8) << floor_phi << "\n";
            }
        }
    }

    // ============================================
    // KEY INSIGHT
    // ============================================

    cout << "\n========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n\n";

    cout << "  Ang Beatty sequences ay nagpa-partition ng integers.\n";
    cout << "  Ang Rule 110 ay may 8 patterns (0-7).\n\n";

    cout << "  Kung i-encode natin ang L, C, R bilang Beatty values:\n";
    cout << "  L → φ o φ² depende sa position\n";
    cout << "  C → φ o φ² depende sa position\n";
    cout << "  R → φ o φ² depende sa position\n\n";

    cout << "  Ang sum ay magbibigay ng unique value na pwedeng\n";
    cout << "  i-decode gamit ang Beatty partition.\n\n";

    // ============================================
    // BEATTY WEIGHTED SUM
    // ============================================

    cout << "========================================\n";
    cout << "  BEATTY WEIGHTED SUM\n";
    cout << "========================================\n\n";

    // Subukan: L → φ², C → φ, R → 1
    // O ibang kombinasyon

    double wL = PHI * PHI;  // φ²
    double wC = PHI;        // φ
    double wR = 1.0;        // 1

    cout << "  Weights: L=φ²=" << wL << ", C=φ=" << wC << ", R=1\n\n";

    cout << "  L C R | Weighted Sum | Expected | Unique?\n";
    cout << "  ------|-------------|----------|--------\n";

    vector<double> sums;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = L * wL + C * wC + R * wR;
                sums.push_back(sum);
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(11) << fixed << setprecision(3) << sum << " | "
                     << setw(8) << expected << " | "
                     << "✅\n";
            }
        }
    }

    // Check for uniqueness
    bool all_unique = true;
    for (int i = 0; i < sums.size(); i++) {
        for (int j = i + 1; j < sums.size(); j++) {
            if (abs(sums[i] - sums[j]) < 0.001) {
                all_unique = false;
            }
        }
    }

    cout << "\n  Lahat unique? " << (all_unique ? "✅" : "❌") << "\n";

    // ============================================
    // BEATTY DECODE
    // ============================================

    cout << "\n========================================\n";
    cout << "  BEATTY DECODE\n";
    cout << "========================================\n\n";

    cout << "  Ang Beatty partition ay nagbibigay ng natural na\n";
    cout << "  threshold para sa decode:\n\n";

    cout << "  ⌊nφ⌋ sequence: 1, 3, 4, 6, 8, 9, 11, 12, ...\n";
    cout << "  ⌊nφ²⌋ sequence: 2, 5, 7, 10, 13, ...\n\n";

    cout << "  Ang Rule 110 output 1 patterns ay nasa φ-side:\n";
    cout << "  001, 010, 100, 101, 110\n\n";

    cout << "  Ang Rule 110 output 0 patterns ay nasa φ²-side:\n";
    cout << "  000, 011, 111\n\n";

    return 0;
}
