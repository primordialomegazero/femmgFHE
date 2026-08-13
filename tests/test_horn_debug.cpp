#include "src/np/golden_exact.h"
#include <iostream>

int main() {
    std::cout << "=== HORN DEBUG ===\n\n";

    // Simpleng Horn: (x1) => x1 = true
    {
        std::vector<std::vector<int>> clauses = {{1}};
        auto result = GoldenExact::GoldenExactSolver::solve(clauses, 1);
        std::cout << "Unit Horn (x1): sat=" << result.satisfiable << "\n";
        if (result.satisfiable) {
            std::cout << "  Assignment: ";
            for (bool v : result.assignment) std::cout << v << " ";
            std::cout << "\n";
        }
    }

    // Horn na may positive: (x1 OR NOT x2 OR NOT x3)
    {
        std::vector<std::vector<int>> clauses = {{1, -2, -3}};
        auto result = GoldenExact::GoldenExactSolver::solve(clauses, 3);
        std::cout << "Horn (x1 OR NOT x2 OR NOT x3): sat=" << result.satisfiable << "\n";
        if (result.satisfiable) {
            std::cout << "  Assignment: ";
            for (bool v : result.assignment) std::cout << v << " ";
            std::cout << "\n";
        }
    }

    // Kilalang satisfiable Horn: (x1) AND (x1 -> x2) AND (x2 -> x3)
    // = (x1) AND (NOT x1 OR x2) AND (NOT x2 OR x3)
    {
        std::vector<std::vector<int>> clauses = {
            {1},          // x1 = true
            {-1, 2},      // x1 -> x2
            {-2, 3}       // x2 -> x3
        };
        auto result = GoldenExact::GoldenExactSolver::solve(clauses, 3);
        std::cout << "Horn chain: sat=" << result.satisfiable << "\n";
        if (result.satisfiable) {
            std::cout << "  Assignment: ";
            for (bool v : result.assignment) std::cout << v << " ";
            std::cout << "\n";
        }
    }

    return 0;
}
