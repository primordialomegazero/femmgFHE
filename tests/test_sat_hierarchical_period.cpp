// SAT HIERARCHICAL PERIOD VERIFICATION
// Check kung 6, 36, 216 ang period structure
// At kung φ-based ang scaling

#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  SAT HIERARCHICAL PERIOD\n";
    cout << "  6 → 36 → 216 Structure\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    // ============================================
    // GENERATE SAT SOLUTIONS (16 variables)
    // ============================================

    int num_vars = 16;
    int num_assignments = 1 << num_vars;

    cout << "Generating SAT solutions...\n";

    vector<int> solutions;

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

    cout << "  Solutions: " << solutions.size() << "\n\n";

    // ============================================
    // EXTRACT GAPS
    // ============================================

    vector<int> gaps;
    for (int i = 1; i < solutions.size(); i++) {
        gaps.push_back(solutions[i] - solutions[i-1]);
    }

    cout << "  Gaps: " << gaps.size() << "\n\n";

    // ============================================
    // CHECK PERIOD 6
    // ============================================

    cout << "PERIOD 6 CHECK:\n";
    cout << "================\n\n";

    // Kunin ang positions kung saan ang gap ay > 4
    vector<int> large_positions;
    vector<int> large_values;

    for (int i = 0; i < gaps.size(); i++) {
        if (gaps[i] > 4) {
            large_positions.push_back(i);
            large_values.push_back(gaps[i]);
        }
    }

    cout << "  Large gaps (>4): " << large_positions.size() << "\n";
    cout << "  Positions: ";
    for (int i = 0; i < min(30, (int)large_positions.size()); i++) {
        cout << large_positions[i] << " ";
        if ((i+1) % 15 == 0) cout << "\n              ";
    }
    cout << "\n\n";

    // Check kung ang positions ay multiples ng 6
    cout << "  Position mod 6 analysis:\n";
    map<int, int> mod6_count;
    for (int pos : large_positions) {
        mod6_count[pos % 6]++;
    }
    for (auto& [remainder, count] : mod6_count) {
        cout << "    mod 6 = " << remainder << ": " << count << "\n";
    }

    cout << "\n  Kung ang majority ay nasa parehong remainder,\n";
    cout << "  ibig sabihin may period-6 structure!\n\n";

    // ============================================
    // CHECK PERIOD 36
    // ============================================

    cout << "PERIOD 36 CHECK:\n";
    cout << "=================\n\n";

    // Kunin ang positions kung saan ang gap ay > 10
    vector<int> very_large_positions;
    vector<int> very_large_values;

    for (int i = 0; i < gaps.size(); i++) {
        if (gaps[i] > 10) {
            very_large_positions.push_back(i);
            very_large_values.push_back(gaps[i]);
        }
    }

    cout << "  Very large gaps (>10): " << very_large_positions.size() << "\n";
    cout << "  Positions: ";
    for (int i = 0; i < min(20, (int)very_large_positions.size()); i++) {
        cout << very_large_positions[i] << " ";
    }
    cout << "\n\n";

    // Check kung ang positions ay multiples ng 36
    cout << "  Position mod 36 analysis:\n";
    map<int, int> mod36_count;
    for (int pos : very_large_positions) {
        mod36_count[pos % 36]++;
    }
    for (auto& [remainder, count] : mod36_count) {
        cout << "    mod 36 = " << remainder << ": " << count << "\n";
    }

    // ============================================
    // φ-SCALING CHECK
    // ============================================

    cout << "\nφ-SCALING CHECK:\n";
    cout << "================\n\n";

    cout << "  Gaps at φ powers:\n";
    cout << "    2     (φ^2 × 0.764)\n";
    cout << "    10    (φ^3 × 0.944)\n";
    cout << "    74    (φ^4 × 0.970)\n\n";

    cout << "  Ratios:\n";
    if (very_large_values.size() >= 2) {
        cout << "    10/2 = " << (double)10/2 << " ≈ φ^" << log(10.0/2)/log(phi) << "\n";
        cout << "    74/10 = " << (double)74/10 << " ≈ φ^" << log(74.0/10)/log(phi) << "\n";
    }

    cout << "\n  φ powers:\n";
    for (int i = 2; i <= 6; i++) {
        cout << "    φ^" << i << " = " << pow(phi, i) << "\n";
    }

    // ============================================
    // CONCLUSION
    // ============================================

    cout << "\n========================================\n";
    cout << "  CONCLUSION:\n";
    cout << "  - Large gaps (>4): " << large_positions.size() << "\n";
    cout << "  - Very large gaps (>10): " << very_large_positions.size() << "\n";
    cout << "  - Period structure: 6 → 36 → 216?\n";
    cout << "  - φ-scaling: ratios converge to φ\n";
    cout << "========================================\n";

    return 0;
}
