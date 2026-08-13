#include "src/np/golden_sat.h"
#include <iostream>
#include <chrono>
#include <random>

int main() {
    std::cout << "=== GOLDEN SAT — LARGE SATISFIABLE ===\n\n";

    for (int num_vars : {10, 20, 30, 40, 50}) {
        int num_clauses = num_vars * 4;
        std::vector<std::vector<int>> clauses;

        std::vector<bool> known_assignment(num_vars, false);
        std::mt19937 rng(12345);
        for (int v = 0; v < num_vars; v++) {
            known_assignment[v] = rng() % 2;
        }

        std::mt19937 rng2(67890);
        for (int c = 0; c < num_clauses; c++) {
            std::vector<int> clause;
            int v1 = rng2() % num_vars;
            int v2 = rng2() % num_vars;
            int v3 = rng2() % num_vars;

            int lit1 = known_assignment[v1] ? (v1 + 1) : -(v1 + 1);
            int lit2 = known_assignment[v2] ? (v2 + 1) : -(v2 + 1);
            int lit3 = known_assignment[v3] ? (v3 + 1) : -(v3 + 1);

            clause.push_back(lit1);
            clause.push_back(lit2);
            clause.push_back(lit3);
            clauses.push_back(clause);
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenSAT::GoldenSATSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "Vars: " << num_vars
                  << " Clauses: " << num_clauses
                  << " Sat: " << result.satisfiable
                  << " Steps: " << result.steps
                  << " Time: " << secs << "s\n";
    }

    return 0;
}
