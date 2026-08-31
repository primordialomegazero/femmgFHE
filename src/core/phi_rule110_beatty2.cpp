// ============================================
// φ-RULE 110 BEATTY2 — DUAL BEATTY SEQUENCES
//
// Subukan ang kombinasyon ng ⌊nφ⌋ at ⌊nφ²⌋
// para ma-perfect ang 8/8 transition
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
    cout << "  φ-RULE 110 BEATTY2 — DUAL SEQUENCES\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // SUBUKAN ANG IBA'T IBANG FLOOR COMBINATIONS
    // ============================================

    cout << "========================================\n";
    cout << "  FLOOR COMBINATIONS\n";
    cout << "========================================\n\n";

    // Subukan: ⌊n×φ⌋ + ⌊n×φ²⌋
    cout << "  COMBO 1: ⌊nφ⌋ + ⌊nφ²⌋\n";
    cout << "  L C R | Binary | ⌊nφ⌋ | ⌊nφ²⌋ | Sum | Sum mod 2 | Expected | Match?\n";
    cout << "  ------|--------|------|-------|-----|-----------|----------|--------\n";

    int match1 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                int f1 = (int)floor(pattern * PHI);
                int f2 = (int)floor(pattern * PHI * PHI);
                int sum = f1 + f2;
                int mod2 = sum % 2;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(6) << pattern << " | "
                     << setw(4) << f1 << " | "
                     << setw(5) << f2 << " | "
                     << setw(3) << sum << " | "
                     << setw(9) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (mod2 == expected ? "✅" : "❌") << "\n";
                
                if (mod2 == expected) match1++;
            }
        }
    }
    cout << "  Match: " << match1 << "/8\n\n";

    // Subukan: ⌊n×φ⌋ × ⌊n×φ²⌋
    cout << "  COMBO 2: ⌊nφ⌋ × ⌊nφ²⌋ mod 2\n";
    cout << "  L C R | ⌊nφ⌋ | ⌊nφ²⌋ | Product | Product mod 2 | Expected | Match?\n";
    cout << "  ------|------|-------|---------|---------------|----------|--------\n";

    int match2 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int pattern = (L << 2) | (C << 1) | R;
                int expected = rule110[pattern];
                
                int f1 = (int)floor(pattern * PHI);
                int f2 = (int)floor(pattern * PHI * PHI);
                int prod = f1 * f2;
                int mod2 = prod % 2;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(4) << f1 << " | "
                     << setw(5) << f2 << " | "
                     << setw(7) << prod << " | "
                     << setw(13) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (mod2 == expected ? "✅" : "❌") << "\n";
                
                if (mod2 == expected) match2++;
            }
        }
    }
    cout << "  Match: " << match2 << "/8\n\n";

    // ============================================
    // SUBUKAN ANG WEIGHTED BEATTY
    // ============================================
    
    cout << "========================================\n";
    cout << "  WEIGHTED BEATTY\n";
    cout << "========================================\n\n";

    // Subukan: L→φ², C→φ, R→1 tapos floor
    cout << "  Weights: L→φ², C→φ, R→1\n";
    cout << "  L C R | L×φ² | C×φ | R×1 | Sum | Floor | Floor mod 2 | Expected | Match?\n";
    cout << "  ------|-------|-----|-----|-----|-------|-------------|----------|--------\n";

    int match3 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                int expected = rule110[(L << 2) | (C << 1) | R];
                
                double val = L * PHI * PHI + C * PHI + R * 1.0;
                int floor_val = (int)floor(val);
                int mod2 = floor_val % 2;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(5) << fixed << setprecision(1) << L * PHI * PHI << " | "
                     << setw(5) << C * PHI << " | "
                     << setw(3) << R << " | "
                     << setw(5) << val << " | "
                     << setw(5) << floor_val << " | "
                     << setw(11) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (mod2 == expected ? "✅" : "❌") << "\n";
                
                if (mod2 == expected) match3++;
            }
        }
    }
    cout << "  Match: " << match3 << "/8\n\n";

    // ============================================
    // SUBUKAN ANG DIFFERENT WEIGHTS
    // ============================================
    
    cout << "========================================\n";
    cout << "  SEARCH PARA SA PERFECT WEIGHTS\n";
    cout << "========================================\n\n";

    cout << "  Hinahanap: L×α + C×β + R×γ na may floor mod 2 = expected\n";
    cout << "  Para sa lahat ng 8 patterns\n\n";

    int best_match = 0;
    double best_alpha = 0, best_beta = 0, best_gamma = 0;
    int best_floor_method = 0;

    for (double alpha : {1.0, PHI, PHI * PHI, 1.0 / PHI, 2.0, 2.0 * PHI}) {
        for (double beta : {1.0, PHI, PHI * PHI, 1.0 / PHI, 2.0, 2.0 * PHI}) {
            for (double gamma : {1.0, PHI, PHI * PHI, 1.0 / PHI, 2.0, 2.0 * PHI}) {
                int matches = 0;
                
                for (int L : {0, 1}) {
                    for (int C : {0, 1}) {
                        for (int R : {0, 1}) {
                            int pattern = (L << 2) | (C << 1) | R;
                            int expected = rule110[pattern];
                            
                            double val = L * alpha + C * beta + R * gamma;
                            int floor_val = (int)floor(val);
                            int mod2 = floor_val % 2;
                            
                            if (mod2 == expected) matches++;
                        }
                    }
                }
                
                if (matches > best_match) {
                    best_match = matches;
                    best_alpha = alpha;
                    best_beta = beta;
                    best_gamma = gamma;
                }
            }
        }
    }

    cout << "  Best match: " << best_match << "/8\n";
    cout << "  Best weights: α=" << best_alpha << ", β=" << best_beta << ", γ=" << best_gamma << "\n\n";

    if (best_match == 8) {
        cout << "  ✅ PERFECT! I-encode ang transition:\n";
        cout << "  L C R | Sum | Floor | mod 2 | Expected\n";
        cout << "  ------|-----|-------|-------|----------\n";
        
        for (int L : {0, 1}) {
            for (int C : {0, 1}) {
                for (int R : {0, 1}) {
                    int pattern = (L << 2) | (C << 1) | R;
                    int expected = rule110[pattern];
                    double val = L * best_alpha + C * best_beta + R * best_gamma;
                    int floor_val = (int)floor(val);
                    int mod2 = floor_val % 2;
                    
                    cout << "  " << L << " " << C << " " << R << " | "
                         << setw(5) << fixed << setprecision(2) << val << " | "
                         << setw(5) << floor_val << " | "
                         << setw(5) << mod2 << " | "
                         << setw(8) << expected << " | ✅\n";
                }
            }
        }
    }

    return 0;
}
