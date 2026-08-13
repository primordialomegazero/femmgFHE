#include "src/np/golden_exact.h"
#include <iostream>
#include <random>

int main() {
    std::cout << "=== P=NP DEBUG ===\n\n";

    int total_tests = 0;
    int total_pass = 0;

    // I-test nang mas detalyado ang bawat kategorya
    auto run_test = [&](const std::string& name, const std::vector<std::vector<int>>& clauses,
                         int num_vars, bool expect_sat) {
        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        total_tests++;
        bool pass = (result.satisfiable == expect_sat);
        if (pass) total_pass++;
        std::cout << name << ": sat=" << result.satisfiable
                  << " expect=" << expect_sat
                  << (pass ? " OK" : " FAIL") << "\n";
    };

    // 2-SAT satisfiable
    std::cout << "--- 2-SAT (satisfiable) ---\n";
    for (int num_vars : {5, 10, 20}) {
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(100 + num_vars);
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

        for (int c = 0; c < num_vars * 3; c++) {
            int v1 = rng() % num_vars + 1;
            int v2 = rng() % num_vars + 1;
            int l1 = known[v1-1] ? v1 : -v1;
            int l2 = known[v2-1] ? v2 : -v2;
            clauses.push_back({l1, l2});
        }
        run_test("  2-SAT vars=" + std::to_string(num_vars), clauses, num_vars, true);
    }

    // 3-SAT satisfiable
    std::cout << "\n--- 3-SAT (satisfiable) ---\n";
    for (int num_vars : {5, 10, 20}) {
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(200 + num_vars);
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

        for (int c = 0; c < num_vars * 4; c++) {
            int v1 = rng() % num_vars + 1;
            int v2 = rng() % num_vars + 1;
            int v3 = rng() % num_vars + 1;
            int l1 = known[v1-1] ? v1 : -v1;
            int l2 = known[v2-1] ? v2 : -v2;
            int l3 = known[v3-1] ? v3 : -v3;
            clauses.push_back({l1, l2, l3});
        }
        run_test("  3-SAT vars=" + std::to_string(num_vars), clauses, num_vars, true);
    }

    // Unsat
    std::cout << "\n--- UNSAT ---\n";
    run_test("  Unit contradiction", {{1}, {-1}}, 1, false);
    run_test("  (x1∨x2)∧(¬x1∨x2)∧(x1∨¬x2)∧(¬x1∨¬x2)", 
             {{1,2}, {-1,2}, {1,-2}, {-1,-2}}, 2, false);

    std::cout << "\nTOTAL: " << total_pass << "/" << total_tests << "\n";
    return 0;
}
