#include "src/np/golden_sat.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "=== GOLDEN SAT SOLVER ===\n\n";

    // Simple SAT: (x1 OR x2) AND (NOT x1 OR x2) AND (x1 OR NOT x2)
    std::vector<std::vector<int>> clauses = {
        {1, 2},      // x1 OR x2
        {-1, 2},     // NOT x1 OR x2
        {1, -2}      // x1 OR NOT x2
    };

    auto result = GoldenSAT::GoldenSATSolver::solve(clauses, 2);

    std::cout << "Satisfiable: " << result.satisfiable << "\n";
    std::cout << "Steps: " << result.steps << "\n";
    std::cout << "Golden score: " << result.golden_score << "\n";

    if (result.satisfiable) {
        std::cout << "Assignment: ";
        for (bool v : result.assignment) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
