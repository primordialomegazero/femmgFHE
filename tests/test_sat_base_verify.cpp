// SAT BASE PATTERN VERIFICATION
// I-verify kung ang period-19 pattern ay universal

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  SAT BASE PATTERN VERIFICATION\n";
    cout << "  Universal Period-19?\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: Same clauses, different variables
    // ============================================
    cout << "TEST 1: Iba't ibang instances\n";
    cout << "=============================\n\n";

    // Function para mag-generate ng solutions
    auto get_solutions = [](int num_vars, vector<vector<int>> clauses) {
        int num_assignments = 1 << num_vars;
        vector<int> solutions;
        for (int i = 0; i < num_assignments; i++) {
            bool sat = true;
            for (auto& clause : clauses) {
                bool clause_sat = false;
                for (int lit : clause) {
                    bool val;
                    if (lit > 0) {
                        val = (i & (1 << (lit - 1))) != 0;
                    } else {
                        val = (i & (1 << (-lit - 1))) == 0;
                    }
                    if (val) {
                        clause_sat = true;
                        break;
                    }
                }
                if (!clause_sat) {
                    sat = false;
                    break;
                }
            }
            if (sat) solutions.push_back(i);
        }
        return solutions;
    };

    // Instance 1: 8 vars, 4 clauses
    auto sol1 = get_solutions(8, {
        {1, 2, 3},
        {-1, 4, 5},
        {2, -3, 6},
        {-4, 7, 8}
    });

    // Instance 2: 8 vars, 5 clauses (iba)
    auto sol2 = get_solutions(8, {
        {1, 2, -3},
        {-1, 4, -5},
        {2, 3, 6},
        {-4, -7, 8},
        {5, -6, 7}
    });

    // Instance 3: 10 vars, 4 clauses
    auto sol3 = get_solutions(10, {
        {1, 2, 3},
        {-2, 5, 7},
        {3, -4, 9},
        {-6, 8, 10}
    });

    cout << "  Instance 1 (8 vars, 4 clauses): " << sol1.size() << " solutions\n";
    cout << "  Instance 2 (8 vars, 5 clauses): " << sol2.size() << " solutions\n";
    cout << "  Instance 3 (10 vars, 4 clauses): " << sol3.size() << " solutions\n\n";

    // Gaps para sa bawat instance
    auto get_gaps = [](vector<int> sol) {
        vector<int> gaps;
        for (size_t i = 1; i < sol.size(); i++) {
            gaps.push_back(sol[i] - sol[i-1]);
        }
        return gaps;
    };

    auto gaps1 = get_gaps(sol1);
    auto gaps2 = get_gaps(sol2);
    auto gaps3 = get_gaps(sol3);

    cout << "  Gaps 1 (first 20): ";
    for (int i = 0; i < min(20, (int)gaps1.size()); i++) cout << gaps1[i] << " ";
    cout << "\n\n";

    cout << "  Gaps 2 (first 20): ";
    for (int i = 0; i < min(20, (int)gaps2.size()); i++) cout << gaps2[i] << " ";
    cout << "\n\n";

    cout << "  Gaps 3 (first 20): ";
    for (int i = 0; i < min(20, (int)gaps3.size()); i++) cout << gaps3[i] << " ";
    cout << "\n\n";

    // ============================================
    // FIBONACCI CHECK
    // ============================================
    vector<int> fib = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144};

    auto fib_count = [&](vector<int> gaps) {
        int count = 0;
        for (int g : gaps) {
            for (int f : fib) {
                if (g == f) {
                    count++;
                    break;
                }
            }
        }
        return count;
    };

    cout << "FIBONACCI GAP ANALYSIS:\n";
    cout << "=======================\n\n";
    cout << "  Instance 1: " << fib_count(gaps1) << "/" << gaps1.size() << "\n";
    cout << "  Instance 2: " << fib_count(gaps2) << "/" << gaps2.size() << "\n";
    cout << "  Instance 3: " << fib_count(gaps3) << "/" << gaps3.size() << "\n\n";

    cout << "========================================\n";
    cout << "  VERDICT: Kung lahat ng instances ay may\n";
    cout << "  >50% Fibonacci gaps, may universal pattern.\n";
    cout << "========================================\n";

    return 0;
}
