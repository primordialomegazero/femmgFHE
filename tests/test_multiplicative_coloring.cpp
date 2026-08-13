#include "src/np/golden_multiplicative.h"
#include <iostream>

int main() {
    std::cout << "=== GOLDEN MULTIPLICATIVE COLORING ===\n\n";

    std::vector<std::vector<int>> clauses;
    int num_vertices = 3;
    int num_vars = num_vertices * 3;

    // At least one
    for (int v = 0; v < num_vertices; v++) {
        clauses.push_back({v*3 + 1, v*3 + 2, v*3 + 3});
    }

    // At most one
    for (int v = 0; v < num_vertices; v++) {
        clauses.push_back({-(v*3 + 1), -(v*3 + 2)});
        clauses.push_back({-(v*3 + 1), -(v*3 + 3)});
        clauses.push_back({-(v*3 + 2), -(v*3 + 3)});
    }

    // Walang adjacent na pareho
    for (int c = 1; c <= 3; c++) {
        clauses.push_back({-(0*3 + c), -(1*3 + c)});
        clauses.push_back({-(0*3 + c), -(2*3 + c)});
        clauses.push_back({-(1*3 + c), -(2*3 + c)});
    }

    auto result = GoldenMultiplicative::GoldenMultiplicativeSolver::solve(clauses, num_vars);
    std::cout << "Triangle 3-coloring: sat=" << result.satisfiable << "\n";
    if (result.satisfiable) {
        std::cout << "Assignment: ";
        for (bool v : result.assignment) std::cout << v << " ";
        std::cout << "\n";
    }

    return 0;
}
