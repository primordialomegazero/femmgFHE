// ============================================
// φ-RULE 110 PHI WEIGHTS — NATURAL φ
//
// Hanapin ang φ-based weights na nagbibigay ng 8/8
// Hindi arbitrary — dapat derived from φ
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 PHI WEIGHTS\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double SQRT_PHI = sqrt(PHI);
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // φ-BASED WEIGHTS
    // ============================================

    cout << "========================================\n";
    cout << "  φ-BASED WEIGHTS SEARCH\n";
    cout << "========================================\n\n";

    // Mga natural na φ-based na values
    vector<double> candidates = {
        PHI_INV,                    // 1/φ = 0.618
        1.0,                        // 1
        PHI_INV + 1.0,              // 1 + 1/φ = φ
        PHI,                        // φ = 1.618
        1.5,                        // 3/2
        PHI - PHI_INV,              // φ - 1/φ = 1
        2.0,                        // 2
        PHI * PHI_INV,              // φ × 1/φ = 1
        (PHI + 1.0) / 2.0,          // (φ+1)/2 = φ²/2
        (PHI + PHI_INV) / 2.0,      // (φ + 1/φ)/2
        (3.0 * PHI) / 2.0,          // 3φ/2
        (PHI + 2.0) / 2.0,          // (φ+2)/2
        (2.0 * PHI + 1.0) / 2.0,    // (2φ+1)/2
        (3.0 + PHI) / 3.0,          // (3+φ)/3
    };

    int best_match = 0;
    double best_a = 0, best_b = 0, best_c = 0;

    for (double a : candidates) {
        for (double b : candidates) {
            for (double c : candidates) {
                int match = 0;
                for (int L : {0, 1}) {
                    for (int C : {0, 1}) {
                        for (int R : {0, 1}) {
                            int pattern = (L << 2) | (C << 1) | R;
                            int expected = rule110[pattern];
                            double val = L * a + C * b + R * c;
                            int floor_val = (int)floor(val);
                            if (floor_val % 2 == expected) match++;
                        }
                    }
                }
                if (match > best_match) {
                    best_match = match;
                    best_a = a;
                    best_b = b;
                    best_c = c;
                }
            }
        }
    }

    cout << "  Best: " << best_match << "/8\n";
    cout << "  α=" << best_a << " (" << best_a / PHI << "×φ)\n";
    cout << "  β=" << best_b << " (" << best_b / PHI << "×φ)\n";
    cout << "  γ=" << best_c << " (" << best_c / PHI << "×φ)\n\n";

    if (best_match == 8) {
        cout << "  ✅ PERFECT φ-BASED 8/8!\n";
        cout << "  L C R | Sum | Floor | mod 2 | Expected\n";
        cout << "  ------|-----|-------|-------|----------\n";
        for (int L : {0, 1}) {
            for (int C : {0, 1}) {
                for (int R : {0, 1}) {
                    int pattern = (L << 2) | (C << 1) | R;
                    int expected = rule110[pattern];
                    double val = L * best_a + C * best_b + R * best_c;
                    int floor_val = (int)floor(val);
                    int mod2 = floor_val % 2;
                    cout << "  " << L << " " << C << " " << R << " | "
                         << setw(5) << fixed << setprecision(3) << val << " | "
                         << setw(5) << floor_val << " | "
                         << setw(5) << mod2 << " | "
                         << setw(8) << expected << " | ✅\n";
                }
            }
        }
    } else {
        cout << "  Walang φ-based weights na 8/8 sa candidates.\n";
        cout << "  Kailangan ng mas maraming φ-based candidates.\n\n";
        
        // Mas malawak na search sa φ-based space
        cout << "  Mas malawak na search...\n\n";
        
        best_match = 0;
        for (int i = 0; i <= 100; i++) {
            for (int j = 0; j <= 100; j++) {
                for (int k = 0; k <= 100; k++) {
                    double a = i * PHI / 100.0 + j * PHI_INV / 100.0;
                    double b = k * PHI / 100.0 + i * PHI_INV / 100.0;
                    double c = j * PHI / 100.0 + k * PHI_INV / 100.0;
                    
                    int match = 0;
                    for (int L : {0, 1}) {
                        for (int C : {0, 1}) {
                            for (int R : {0, 1}) {
                                int pattern = (L << 2) | (C << 1) | R;
                                int expected = rule110[pattern];
                                double val = L * a + C * b + R * c;
                                int floor_val = (int)floor(val);
                                if (floor_val % 2 == expected) match++;
                            }
                        }
                    }
                    if (match > best_match) {
                        best_match = match;
                        best_a = a;
                        best_b = b;
                        best_c = c;
                    }
                    if (match == 8) break;
                }
                if (best_match == 8) break;
            }
            if (best_match == 8) break;
        }
        
        cout << "  Best: " << best_match << "/8\n";
        cout << "  α=" << best_a << "\n";
        cout << "  β=" << best_b << "\n";
        cout << "  γ=" << best_c << "\n\n";
        
        if (best_match == 8) {
            cout << "  ✅ PERFECT φ-BASED 8/8!\n";
            cout << "  L C R | Sum | Floor | mod 2 | Expected\n";
            cout << "  ------|-----|-------|-------|----------\n";
            for (int L : {0, 1}) {
                for (int C : {0, 1}) {
                    for (int R : {0, 1}) {
                        int pattern = (L << 2) | (C << 1) | R;
                        int expected = rule110[pattern];
                        double val = L * best_a + C * best_b + R * best_c;
                        int floor_val = (int)floor(val);
                        int mod2 = floor_val % 2;
                        cout << "  " << L << " " << C << " " << R << " | "
                             << setw(5) << fixed << setprecision(3) << val << " | "
                             << setw(5) << floor_val << " | "
                             << setw(5) << mod2 << " | "
                             << setw(8) << expected << " | ✅\n";
                    }
                }
            }
        }
    }

    return 0;
}
