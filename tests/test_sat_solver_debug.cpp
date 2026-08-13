#include "src/np/golden_sat.h"
#include <iostream>

int main() {
    std::cout << "=== SAT SOLVER DEBUG ===\n\n";

    std::vector<std::vector<int>> clauses = {
        {1, 2},
        {-1, 2},
        {1, -2}
    };

    auto result = GoldenSAT::GoldenSATSolver::solve(clauses, 2);

    std::cout << "Satisfiable: " << result.satisfiable << "\n";
    std::cout << "Steps: " << result.steps << "\n";
    std::cout << "Score: " << result.golden_score << "\n";
    std::cout << "Assignment: ";
    for (bool v : result.assignment) {
        std::cout << v << " ";
    }
    std::cout << "\n";

    return 0;
}
