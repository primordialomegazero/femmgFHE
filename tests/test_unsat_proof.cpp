#include "src/np/golden_sat.h"
#include <iostream>
#include <chrono>
#include <random>
#include <set>

int main() {
    std::cout << "=== UNSAT CERTIFICATION ===\n\n";

    // Test: Kilalang unsat instances na may polynomial-size proof
    for (int num_vars : {10, 20, 30, 50}) {
        // Pigeonhole principle: n+1 pigeons sa n holes = unsat
        int holes = num_vars;
        int pigeons = num_vars + 1;
        std::vector<std::vector<int>> clauses;

        // Bawat pigeon ay nasa kahit isang hole
        for (int p = 0; p < pigeons; p++) {
            std::vector<int> clause;
            for (int h = 0; h < holes; h++) {
                clause.push_back(p * holes + h + 1);
            }
            clauses.push_back(clause);
        }

        // Walang dalawang pigeon sa parehong hole
        for (int h = 0; h < holes; h++) {
            for (int p1 = 0; p1 < pigeons; p1++) {
                for (int p2 = p1 + 1; p2 < pigeons; p2++) {
                    clauses.push_back({-(p1 * holes + h + 1), -(p2 * holes + h + 1)});
                }
            }
        }

        int total_vars = holes * pigeons;
        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenSAT::GoldenSATSolver::solve(clauses, total_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "Pigeonhole (" << holes << " holes, " << pigeons << " pigeons):\n";
        std::cout << "  Vars: " << total_vars
                  << " Clauses: " << clauses.size()
                  << " Sat: " << result.satisfiable
                  << " (expect 0 - unsat)\n";
        std::cout << "  Steps: " << result.steps
                  << " Time: " << secs << "s\n\n";
    }

    return 0;
}
