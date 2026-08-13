#include "src/np/golden_prime_cardinality.h"
#include <iostream>

int main() {
    std::cout << "=== GOLDEN PRIME GRAPH COLORING ===\n\n";

    // Triangle (3 vertices, 3 colors)
    std::vector<std::vector<int>> clauses;
    int num_vertices = 3;
    int num_vars = num_vertices * 3;

    // At least one color per vertex
    for (int v = 0; v < num_vertices; v++) {
        clauses.push_back({v*3 + 1, v*3 + 2, v*3 + 3});
    }

    // At most one color per vertex
    for (int v = 0; v < num_vertices; v++) {
        clauses.push_back({-(v*3 + 1), -(v*3 + 2)});
        clauses.push_back({-(v*3 + 1), -(v*3 + 3)});
        clauses.push_back({-(v*3 + 2), -(v*3 + 3)});
    }

    // Walang adjacent vertices na pareho ang kulay
    for (int c = 1; c <= 3; c++) {
        clauses.push_back({-(0*3 + c), -(1*3 + c)});
        clauses.push_back({-(0*3 + c), -(2*3 + c)});
        clauses.push_back({-(1*3 + c), -(2*3 + c)});
    }

    auto result = GoldenPrimeCardinality::GoldenPrimeSolver::solve(clauses, num_vars);
    std::cout << "Triangle 3-coloring: sat=" << result.satisfiable << "\n";
    if (result.satisfiable) {
        std::cout << "Assignment: ";
        for (bool v : result.assignment) std::cout << v << " ";
        std::cout << "\n";
    }

    return 0;
}
