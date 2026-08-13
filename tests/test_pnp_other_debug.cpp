#include "src/np/golden_exact.h"
#include <iostream>
#include <random>

int main() {
    std::cout << "=== OTHER SAT VARIANTS DEBUG ===\n\n";

    int total_tests = 0;
    int total_pass = 0;

    // 4-SAT
    std::cout << "--- 4-SAT ---\n";
    for (int num_vars : {5, 10, 20}) {
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(500 + num_vars);
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

        for (int c = 0; c < num_vars * 3; c++) {
            std::vector<int> clause;
            for (int k = 0; k < 4; k++) {
                int v = rng() % num_vars + 1;
                int l = known[v-1] ? v : -v;
                clause.push_back(l);
            }
            clauses.push_back(clause);
        }

        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        total_tests++;
        bool pass = result.satisfiable;
        if (pass) total_pass++;
        std::cout << "  4-SAT vars=" << num_vars << ": sat=" << result.satisfiable
                  << (pass ? " OK" : " FAIL") << "\n";
    }

    // 5-SAT
    std::cout << "\n--- 5-SAT ---\n";
    for (int num_vars : {5, 10}) {
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(600 + num_vars);
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

        for (int c = 0; c < num_vars * 2; c++) {
            std::vector<int> clause;
            for (int k = 0; k < 5; k++) {
                int v = rng() % num_vars + 1;
                int l = known[v-1] ? v : -v;
                clause.push_back(l);
            }
            clauses.push_back(clause);
        }

        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        total_tests++;
        bool pass = result.satisfiable;
        if (pass) total_pass++;
        std::cout << "  5-SAT vars=" << num_vars << ": sat=" << result.satisfiable
                  << (pass ? " OK" : " FAIL") << "\n";
    }

    // Horn SAT
    std::cout << "\n--- Horn SAT ---\n";
    for (int num_vars : {5, 10, 20}) {
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(700 + num_vars);
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

        for (int c = 0; c < num_vars * 3; c++) {
            std::vector<int> clause;
            int pos_idx = rng() % 3;
            for (int j = 0; j < 3; j++) {
                int v = rng() % num_vars + 1;
                if (j == pos_idx && known[v-1]) {
                    clause.push_back(v);
                } else {
                    clause.push_back(-v);
                }
            }
            clauses.push_back(clause);
        }

        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        total_tests++;
        bool pass = result.satisfiable;
        if (pass) total_pass++;
        std::cout << "  Horn vars=" << num_vars << ": sat=" << result.satisfiable
                  << (pass ? " OK" : " FAIL") << "\n";
    }

    std::cout << "\nTOTAL: " << total_pass << "/" << total_tests << "\n";
    return 0;
}
