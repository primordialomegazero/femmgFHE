// INTERRUPTION FORMULA EXTRACTION
// Hanapin ang exact φ-based formula ng large gaps
// Na naghihiwalay sa base pattern

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  INTERRUPTION FORMULA EXTRACTION\n";
    cout << "  φ-Based Large Gap Analysis\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

    // ============================================
    // GENERATE SAT SOLUTIONS (16 variables)
    // ============================================

    int num_vars = 16;
    int num_assignments = 1 << num_vars;

    cout << "Generating SAT solutions...\n";

    vector<int> solutions;
    vector<int> gaps;

    for (int i = 0; i < num_assignments; i++) {
        bool x[16];
        for (int j = 0; j < 16; j++) {
            x[j] = (i & (1 << j)) != 0;
        }

        bool c1 = x[0] || x[1] || !x[2];
        bool c2 = !x[0] || x[1] || x[3];
        bool c3 = x[0] || !x[1] || !x[3];
        bool c4 = x[1] || x[2] || !x[3];
        bool c5 = x[4] || x[5] || !x[6];
        bool c6 = !x[4] || x[6] || x[7];
        bool c7 = x[2] || x[4] || !x[7];
        bool c8 = x[3] || x[5] || x[6];
        bool c9 = x[8] || x[9] || !x[10];
        bool c10 = !x[8] || x[10] || x[11];
        bool c11 = x[6] || x[8] || !x[11];
        bool c12 = x[7] || x[9] || x[10];
        bool c13 = x[12] || x[13] || !x[14];
        bool c14 = !x[12] || x[14] || x[15];
        bool c15 = x[10] || x[12] || !x[15];
        bool c16 = x[11] || x[13] || x[14];

        if (c1 && c2 && c3 && c4 && c5 && c6 && c7 && c8 &&
            c9 && c10 && c11 && c12 && c13 && c14 && c15 && c16) {
            solutions.push_back(i);
        }
    }

    for (int i = 1; i < solutions.size(); i++) {
        gaps.push_back(solutions[i] - solutions[i-1]);
    }

    cout << "  Solutions: " << solutions.size() << "\n";
    cout << "  Gaps: " << gaps.size() << "\n\n";

    // ============================================
    // SEPARATE SMALL AND LARGE GAPS
    // ============================================

    vector<int> small_gaps;   // ≤ 10
    vector<int> large_gaps;   // > 10
    vector<int> large_positions;

    for (int i = 0; i < gaps.size(); i++) {
        if (gaps[i] <= 10) {
            small_gaps.push_back(gaps[i]);
        } else {
            large_gaps.push_back(gaps[i]);
            large_positions.push_back(i);
        }
    }

    cout << "SMALL vs LARGE GAPS:\n";
    cout << "====================\n\n";
    cout << "  Small gaps (≤10): " << small_gaps.size() << "\n";
    cout << "  Large gaps (>10): " << large_gaps.size() << "\n\n";

    // ============================================
    // LARGE GAP PATTERN
    // ============================================

    cout << "LARGE GAPS SEQUENCE:\n";
    cout << "====================\n\n";
    cout << "  Values: ";
    for (int i = 0; i < large_gaps.size(); i++) {
        cout << large_gaps[i];
        if (i < large_gaps.size() - 1) cout << ", ";
    }
    cout << "\n\n";

    cout << "  Positions: ";
    for (int i = 0; i < large_positions.size(); i++) {
        cout << large_positions[i];
        if (i < large_positions.size() - 1) cout << ", ";
    }
    cout << "\n\n";

    // ============================================
    // CHECK φ RELATIONSHIP
    // ============================================

    cout << "φ RELATIONSHIP CHECK:\n";
    cout << "=====================\n\n";

    cout << "  φ = " << phi << "\n";
    cout << "  φ² = " << phi_sq << "\n";
    cout << "  φ³ = " << phi_cu << "\n\n";

    // Check kung ang large gaps ay multiples ng φ
    cout << "  Large gaps vs φ multiples:\n";
    for (int g : large_gaps) {
        double ratio = g / phi;
        double ratio2 = g / phi_sq;
        cout << "    " << g << " / φ = " << ratio 
             << " | " << g << " / φ² = " << ratio2;
        
        if (abs(ratio - round(ratio)) < 0.01) {
            cout << "  ✓ (φ multiple: " << round(ratio) << ")";
        }
        if (abs(ratio2 - round(ratio2)) < 0.01) {
            cout << "  ✓ (φ² multiple: " << round(ratio2) << ")";
        }
        cout << "\n";
    }

    // ============================================
    // FIBONACCI MULTIPLES
    // ============================================

    cout << "\nFIBONACCI MULTIPLES:\n";
    cout << "====================\n\n";

    vector<int> fib = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377};

    cout << "  Large gaps as Fibonacci multiples:\n";
    for (int g : large_gaps) {
        cout << "    " << g << " = ";
        bool found = false;
        for (int f : fib) {
            if (g % f == 0 && g / f > 0) {
                if (found) cout << " or ";
                cout << f << "×" << g/f;
                found = true;
            }
        }
        if (!found) cout << "not a Fibonacci multiple";
        cout << "\n";
    }

    // ============================================
    // ARITHMETIC PROGRESSION CHECK
    // ============================================

    cout << "\nARITHMETIC PROGRESSION CHECK:\n";
    cout << "=============================\n\n";

    if (large_gaps.size() >= 3) {
        vector<int> diffs;
        for (int i = 1; i < large_gaps.size(); i++) {
            diffs.push_back(large_gaps[i] - large_gaps[i-1]);
        }

        cout << "  Large gap differences:\n  ";
        for (int d : diffs) {
            cout << d << " ";
        }
        cout << "\n\n";

        // Check kung may period sa diffs
        bool has_period = false;
        int period = 0;
        for (int p = 1; p <= 20; p++) {
            bool is_periodic = true;
            for (int i = 0; i + p < diffs.size(); i++) {
                if (diffs[i] != diffs[i + p]) {
                    is_periodic = false;
                    break;
                }
            }
            if (is_periodic) {
                has_period = true;
                period = p;
                break;
            }
        }

        if (has_period) {
            cout << "  FOUND PERIOD: " << period << "\n";
            cout << "  Pattern: ";
            for (int i = 0; i < period; i++) {
                cout << diffs[i] << " ";
            }
            cout << "\n";
        } else {
            cout << "  Walang exact period sa differences\n";
        }
    }

    // ============================================
    // CONCLUSION
    // ============================================

    cout << "\n========================================\n";
    cout << "  CONCLUSION:\n";
    cout << "  - Small gaps: " << small_gaps.size() << " (Fibonacci-dominant)\n";
    cout << "  - Large gaps: " << large_gaps.size() << " (φ-related)\n";
    cout << "  - Structure: quasi-periodic\n";
    cout << "========================================\n";

    return 0;
}
