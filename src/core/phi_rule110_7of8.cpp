// ============================================
// φ-RULE 110 7/8 — ISANG MALI NA LANG
//
// Best weights: α=φ, β=1, γ=φ
// Kailangan malaman kung alin ang sumablay
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
    cout << "  φ-RULE 110 7/8 ANALYSIS\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    double alpha = PHI;  // ≈ 1.618
    double beta = 1.0;
    double gamma = PHI;  // ≈ 1.618

    cout << "  Weights: L→φ=" << alpha << ", C→1=" << beta << ", R→φ=" << gamma << "\n\n";

    cout << "  L C R | Weighted Sum | Floor | mod 2 | Expected | Match?\n";
    cout << "  ------|-------------|-------|-------|----------|--------\n";

    int matches = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                double val = L * alpha + C * beta + R * gamma;
                int floor_val = (int)floor(val);
                int mod2 = floor_val % 2;
                bool match = (mod2 == expected);
                if (match) matches++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(11) << fixed << setprecision(3) << val << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << matches << "/8\n\n";

    // ============================================
    // ANO ANG SUMABLAY?
    // ============================================

    cout << "========================================\n";
    cout << "  ANO ANG SUMABLAY?\n";
    cout << "========================================\n\n";

    // Hanapin kung alin ang mali
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                double val = L * alpha + C * beta + R * gamma;
                int floor_val = (int)floor(val);
                int mod2 = floor_val % 2;
                
                if (mod2 != expected) {
                    cout << "  ❌ Pattern " << L << C << R << " (binary " << pattern << ")\n";
                    cout << "     Weighted sum: " << fixed << setprecision(10) << val << "\n";
                    cout << "     Floor: " << floor_val << "\n";
                    cout << "     Mod 2: " << mod2 << "\n";
                    cout << "     Expected: " << expected << "\n";
                    
                    cout << "\n     Kailangan: baguhin ang value para magbago ang mod 2\n";
                    cout << "     Current: floor = " << floor_val << ", mod 2 = " << mod2 << "\n";
                    cout << "     Target: mod 2 = " << expected << "\n";
                    
                    if (mod2 == 0 && expected == 1) {
                        cout << "     Kailangan: floor maging ODD\n";
                        cout << "     Current floor: " << floor_val << " (EVEN)\n";
                        cout << "     Next odd floor: " << floor_val + 1 << "\n";
                        cout << "     Previous odd floor: " << floor_val - 1 << "\n\n";
                    } else if (mod2 == 1 && expected == 0) {
                        cout << "     Kailangan: floor maging EVEN\n";
                        cout << "     Current floor: " << floor_val << " (ODD)\n";
                        cout << "     Next even floor: " << floor_val + 1 << "\n";
                        cout << "     Previous even floor: " << floor_val - 1 << "\n\n";
                    }
                }
            }
        }
    }

    // ============================================
    // POSSIBLE FIX
    // ============================================

    cout << "========================================\n";
    cout << "  POSSIBLE FIX\n";
    cout << "========================================\n\n";

    cout << "  Kailangan natin ng weighting na magbibigay ng:\n";
    cout << "  - Lahat ng expected=1 patterns ay ODD floor\n";
    cout << "  - Lahat ng expected=0 patterns ay EVEN floor\n\n";

    cout << "  Expected=1: 001, 010, 100, 101, 110\n";
    cout << "  Expected=0: 000, 011, 111\n\n";

    cout << "  Subukan natin ang mas precise na weights:\n";
    cout << "  Sa halip na φ≈1.618, gamitin ang eksaktong φ\n\n";

    // Mas precise na paghahanap
    cout << "  Precise search...\n\n";

    int best_match = 0;
    double best_a = 0, best_b = 0, best_c = 0;

    for (double a = 1.0; a <= 3.0; a += 0.001) {
        for (double b = 0.5; b <= 2.0; b += 0.001) {
            for (double c = 1.0; c <= 3.0; c += 0.001) {
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

    cout << "  Best: " << best_match << "/8 with α=" << best_a << ", β=" << best_b << ", γ=" << best_c << "\n\n";

    if (best_match == 8) {
        cout << "  ✅ PERFECT 8/8!\n";
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
        cout << "  Walang perfect weights sa range na ito.\n";
        cout << "  Kailangan ng ibang approach.\n\n";
    }

    return 0;
}
