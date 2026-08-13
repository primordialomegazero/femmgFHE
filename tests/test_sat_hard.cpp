#include "src/np/golden_sat.h"
#include <iostream>
#include <chrono>
#include <random>
#include <vector>

int main() {
    std::cout << "=== GOLDEN SAT — HARD INSTANCES ===\n\n";

    // Phase transition: clause/variable ratio ≈ 4.26
    // Ito ang pinakamahirap na SAT instances
    for (int num_vars : {50, 100, 150, 200, 300}) {
        // 4.26 clauses per variable
        int num_clauses = (int)(num_vars * 4.26);

        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(std::random_device{}());

        for (int c = 0; c < num_clauses; c++) {
            std::vector<int> clause;
            // 3 random variables na walang duplicate
            int v1 = rng() % num_vars + 1;
            int v2 = rng() % num_vars + 1;
            int v3 = rng() % num_vars + 1;

            // Random signs
            if (rng() % 2) v1 = -v1;
            if (rng() % 2) v2 = -v2;
            if (rng() % 2) v3 = -v3;

            clause.push_back(v1);
            clause.push_back(v2);
            clause.push_back(v3);
            clauses.push_back(clause);
        }

        std::cout << "Vars: " << num_vars << " Clauses: " << num_clauses << "\n";

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenSAT::GoldenSATSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "  Sat: " << result.satisfiable
                  << " Steps: " << result.steps
                  << " Time: " << secs << "s"
                  << " Score: " << result.golden_score << "\n\n";
    }

    return 0;
}
