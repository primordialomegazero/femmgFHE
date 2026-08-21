// GAP PATTERN VERIFICATION
// Check kung periodic ang solution gaps
// At kung kaya nang i-compute directly

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  GAP PATTERN VERIFICATION\n";
    cout << "  Periodic Structure Check\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    // ============================================
    // GENERATE SAT SOLUTIONS (16 variables)
    // ============================================

    int num_vars = 16;
    int num_assignments = 1 << num_vars; // 65536

    cout << "Generating SAT solutions (" << num_vars << " variables)...\n";

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
    // EXTRACT GAP SEQUENCE
    // ============================================

    vector<int> gaps;
    for (int i = 1; i < solutions.size(); i++) {
        gaps.push_back(solutions[i] - solutions[i-1]);
    }

    cout << "GAP SEQUENCE (first 100):\n";
    cout << "=========================\n\n";
    for (int i = 0; i < min(100, (int)gaps.size()); i++) {
        cout << gaps[i];
        if (i < min(100, (int)gaps.size()) - 1) cout << ", ";
        if ((i + 1) % 15 == 0) cout << "\n";
    }
    cout << "\n\n";

    // ============================================
    // FIND PERIODICITY
    // ============================================

    cout << "PERIODICITY CHECK:\n";
    cout << "==================\n\n";

    // Find smallest period
    bool found_period = false;
    int period = 0;

    for (int p = 1; p <= 50; p++) {
        bool is_periodic = true;
        for (int i = 0; i + p < gaps.size(); i++) {
            if (gaps[i] != gaps[i + p]) {
                is_periodic = false;
                break;
            }
        }
        if (is_periodic) {
            found_period = true;
            period = p;
            break;
        }
    }

    if (found_period) {
        cout << "  FOUND PERIOD: " << period << "\n";
        cout << "  Pattern: ";
        for (int i = 0; i < period; i++) {
            cout << gaps[i];
            if (i < period - 1) cout << ", ";
        }
        cout << "\n\n";
    } else {
        cout << "  Walang exact period na nakita (≤50)\n\n";
    }

    // ============================================
    // PARTIAL PERIODICITY
    // ============================================

    cout << "PARTIAL PERIODICITY ANALYSIS:\n";
    cout << "=============================\n\n";

    // Check kung may base pattern na may occasional large gaps
    vector<int> small_gaps;
    vector<int> large_gaps;
    vector<int> large_positions;

    for (int i = 0; i < gaps.size(); i++) {
        if (gaps[i] <= 10) {
            small_gaps.push_back(gaps[i]);
        } else {
            large_gaps.push_back(gaps[i]);
            large_positions.push_back(i);
        }
    }

    cout << "  Small gaps (≤10): " << small_gaps.size() << "\n";
    cout << "  Large gaps (>10): " << large_gaps.size() << "\n";
    cout << "  Large gap positions: ";
    for (int pos : large_positions) {
        cout << pos << " ";
        if (large_positions.size() > 20 && pos > 20) break;
    }
    cout << "\n\n";

    // Check pattern ng small gaps
    if (small_gaps.size() > 10) {
        cout << "  Small gap pattern (first 50): ";
        for (int i = 0; i < min(50, (int)small_gaps.size()); i++) {
            cout << small_gaps[i];
            if (i < min(50, (int)small_gaps.size()) - 1) cout << ",";
        }
        cout << "\n\n";
    }

    // Check large gaps progression
    if (large_gaps.size() > 3) {
        cout << "  Large gaps: ";
        for (int i = 0; i < min(20, (int)large_gaps.size()); i++) {
            cout << large_gaps[i];
            if (i < min(20, (int)large_gaps.size()) - 1) cout << ", ";
        }
        cout << "\n\n";

        // Check differences ng large gaps
        vector<int> large_diffs;
        for (int i = 1; i < large_gaps.size(); i++) {
            large_diffs.push_back(large_gaps[i] - large_gaps[i-1]);
        }
        cout << "  Large gap differences: ";
        for (int i = 0; i < min(20, (int)large_diffs.size()); i++) {
            cout << large_diffs[i];
            if (i < min(20, (int)large_diffs.size()) - 1) cout << ", ";
        }
        cout << "\n\n";
    }

    // ============================================
    // DIRECT FORMULA
    // ============================================

    cout << "DIRECT FORMULA:\n";
    cout << "===============\n\n";

    if (solutions.size() >= 3) {
        // Check kung ang first few solutions ay may pattern
        cout << "  First 20 solutions: ";
        for (int i = 0; i < min(20, (int)solutions.size()); i++) {
            cout << solutions[i] << " ";
        }
        cout << "\n\n";

        // Try: solution_i = a*i + b (linear)
        if (solutions.size() >= 2) {
            double a = solutions[1] - solutions[0];
            double b = solutions[0];
            cout << "  Linear: sol_i = " << a << "*i + " << b << "\n";

            int matches = 0;
            for (int i = 0; i < min(100, (int)solutions.size()); i++) {
                if (abs(a*i + b - solutions[i]) < 1e-6) matches++;
            }
            cout << "  Matches: " << matches << "/100\n\n";
        }

        // Try: solution_i = i*2 + offset
        cout << "  Try sol_i = 2*i + offset:\n";
        int best_offset = 0;
        int best_matches = 0;
        for (int offset = 0; offset < 20; offset++) {
            int matches = 0;
            for (int i = 0; i < min(50, (int)solutions.size()); i++) {
                if (2*i + offset == solutions[i]) matches++;
            }
            if (matches > best_matches) {
                best_matches = matches;
                best_offset = offset;
            }
        }
        cout << "    Best offset: " << best_offset << " (matches: " << best_matches << "/50)\n\n";
    }

    // ============================================
    // CONCLUSION
    // ============================================

    cout << "========================================\n";
    cout << "  CONCLUSION:\n";
    cout << "  - Solutions: " << solutions.size() << "\n";
    cout << "  - Gaps: " << gaps.size() << "\n";
    if (found_period) {
        cout << "  - Period: " << period << "\n";
        cout << "  - Direct formula possible!\n";
    } else {
        cout << "  - No exact period\n";
        cout << "  - May have quasi-periodic structure\n";
    }
    cout << "========================================\n";

    return 0;
}
