#include "src/np/golden_exact.h"
#include <iostream>
#include <random>

int main() {
    std::cout << "=== LARGE INSTANCE DEBUG ===\n\n";

    int total_tests = 0;
    int total_pass = 0;

    // 3-SAT satisfiable na may iba't ibang laki
    for (int num_vars : {100, 200, 500, 1000, 2000, 5000}) {
        int num_clauses = num_vars * 4;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(42);
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

        for (int c = 0; c < num_clauses; c++) {
            int v1 = rng() % num_vars + 1;
            int v2 = rng() % num_vars + 1;
            int v3 = rng() % num_vars + 1;
            int l1 = known[v1-1] ? v1 : -v1;
            int l2 = known[v2-1] ? v2 : -v2;
            int l3 = known[v3-1] ? v3 : -v3;
            clauses.push_back({l1, l2, l3});
        }

        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        total_tests++;
        bool pass = result.satisfiable;
        if (pass) total_pass++;

        // Kung nag-fail, i-print ang score at assignment
        if (!pass) {
            std::cout << "FAIL: vars=" << num_vars
                      << " score=" << result.golden_score
                      << " steps=" << result.steps << "\n";
        } else {
            std::cout << "OK: vars=" << num_vars << "\n";
        }
    }

    std::cout << "\nTOTAL: " << total_pass << "/" << total_tests << "\n";
    return 0;
}
