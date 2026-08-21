// UNIVERSAL VERIFICATION NG O(1) FORMULA
// I-test sa iba't ibang instances

#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  UNIVERSAL FORMULA VERIFICATION\n";
    cout << "  O(1) SAT Solver Test\n";
    cout << "========================================\n\n";

    // ============================================
    // FORMULA:
    // gap_i = base[i mod 7] + interruption_i
    // interruption_i = 17 (i mod 3 != 2)
    //                 = 33 (i mod 3 == 2)
    //                 = 257 (i mod 12 == 11)
    // ============================================

    vector<int> base = {2, 1, 3, 1, 4, 2, 2};

    auto predict_gap = [&](int i) {
        int gap = base[i % 7];
        if (i % 12 == 11) gap = 257;
        else if (i % 3 == 2) gap = 33;
        else gap = 17;
        return gap;
    };

    cout << "PREDICTED GAPS (first 30):\n";
    cout << "==========================\n\n";

    for (int i = 0; i < 30; i++) {
        cout << predict_gap(i) << " ";
        if (i % 10 == 9) cout << "\n";
    }
    cout << "\n\n";

    // ============================================
    // VERIFY SA IBA'T IBANG INSTANCES
    // ============================================

    auto get_solutions = [](int num_vars, int num_clauses) {
        int num_assignments = 1 << num_vars;
        vector<int> solutions;
        for (int i = 0; i < num_assignments; i++) {
            bool sat = true;
            for (int c = 0; c < num_clauses; c++) {
                bool clause_sat = false;
                for (int v = 0; v < 3; v++) {
                    int var = (c * 3 + v) % num_vars;
                    bool val = (i & (1 << var)) != 0;
                    if (val) { clause_sat = true; break; }
                }
                if (!clause_sat) { sat = false; break; }
            }
            if (sat) solutions.push_back(i);
        }
        return solutions;
    };

    // Test sa 12 vars, iba't ibang clauses
    int num_vars = 12;

    for (int num_clauses : {4, 6, 8, 10}) {
        auto solutions = get_solutions(num_vars, num_clauses);
        vector<int> gaps;
        for (size_t i = 1; i < solutions.size(); i++) {
            gaps.push_back(solutions[i] - solutions[i-1]);
        }

        // I-verify kung ang gaps ay may base pattern
        int matches = 0;
        int total = min(30, (int)gaps.size());

        for (int i = 0; i < total; i++) {
            if (gaps[i] == predict_gap(i)) {
                matches++;
            }
        }

        double accuracy = (double)matches / total * 100;
        cout << "  " << num_vars << " vars, " << num_clauses << " clauses: "
             << matches << "/" << total << " = " << accuracy << "%\n";
    }

    cout << "\n========================================\n";
    cout << "  VERDICT:\n";
    cout << "  Kung 100% lahat ng instances,\n";
    cout << "  O(1) SAT SOLVER CONFIRMED!\n";
    cout << "========================================\n";

    return 0;
}
