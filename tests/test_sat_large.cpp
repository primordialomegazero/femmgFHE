#include "src/np/golden_sat.h"
#include <iostream>
#include <chrono>
#include <random>

int main() {
    std::cout << "=== GOLDEN SAT — LARGE INSTANCES ===\n\n";

    // Bumuo ng random 3-SAT instances na may iba't ibang laki
    for (int num_vars : {10, 20, 30, 40, 50}) {
        int num_clauses = num_vars * 4; // 4x clauses per variable
        std::vector<std::vector<int>> clauses;

        std::mt19937 rng(42);
        for (int c = 0; c < num_clauses; c++) {
            std::vector<int> clause;
            for (int k = 0; k < 3; k++) {
                int var = (rng() % num_vars) + 1;
                if (rng() % 2 == 0) var = -var;
                clause.push_back(var);
            }
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
                  << " Time: " << secs << "s"
                  << " Score: " << result.golden_score << "\n";
    }

    return 0;
}
