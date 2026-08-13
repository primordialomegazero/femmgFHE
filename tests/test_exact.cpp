#include "src/np/golden_exact.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== GOLDEN EXACT SAT ===\n\n";

    // Test 1: Simpleng satisfiable
    {
        std::vector<std::vector<int>> clauses = {
            {1, 2},
            {-1, 2},
            {1, -2}
        };
        auto result = GoldenExact::GoldenExactSolver::solve(clauses, 2);
        std::cout << "Simple SAT: sat=" << result.satisfiable
                  << " score=" << result.golden_score << "\n";
        if (result.satisfiable) {
            std::cout << "  Assignment: ";
            for (bool v : result.assignment) std::cout << v << " ";
            std::cout << "\n";
        }
    }

    // Test 2: Simpleng unsat
    {
        std::vector<std::vector<int>> clauses = {{1}, {-1}};
        auto result = GoldenExact::GoldenExactSolver::solve(clauses, 1);
        std::cout << "Simple UNSAT: sat=" << result.satisfiable << "\n";
    }

    // Test 3: Random satisfiable
    std::cout << "\n--- Random Satisfiable ---\n";
    for (int num_vars : {10, 20, 50, 100}) {
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

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "  Vars=" << num_vars << ": sat=" << result.satisfiable
                  << " score=" << result.golden_score
                  << " time=" << secs << "s\n";
    }

    return 0;
}
