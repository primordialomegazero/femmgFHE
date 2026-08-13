#include "src/np/golden_exact_gates.h"
#include <iostream>
#include <random>

int main() {
    std::cout << "=== GRAPH COLORING DEBUG ===\n\n";

    // Simpleng triangle (3 vertices, fully connected)
    // 3-colorable: Oo
    std::vector<std::vector<int>> graph = {
        {1, 2},      // vertex 0 connected sa 1 at 2
        {0, 2},      // vertex 1 connected sa 0 at 2
        {0, 1}       // vertex 2 connected sa 0 at 1
    };

    int num_vertices = 3;
    int num_vars = num_vertices * 3;
    std::vector<std::vector<int>> clauses;

    // Bawat vertex ay may eksaktong isang kulay
    for (int v = 0; v < num_vertices; v++) {
        // At least one: (c1 OR c2 OR c3)
        clauses.push_back({v*3 + 1, v*3 + 2, v*3 + 3});
        // At most one: mga pairwise negations
        clauses.push_back({-(v*3 + 1), -(v*3 + 2)});
        clauses.push_back({-(v*3 + 1), -(v*3 + 3)});
        clauses.push_back({-(v*3 + 2), -(v*3 + 3)});
    }

    // Walang dalawang adjacent vertices na pareho ang kulay
    for (int v1 = 0; v1 < num_vertices; v1++) {
        for (int v2 : graph[v1]) {
            if (v1 < v2) {
                for (int c = 1; c <= 3; c++) {
                    clauses.push_back({-(v1*3 + c), -(v2*3 + c)});
                }
            }
        }
    }

    std::cout << "Clauses: " << clauses.size() << "\n";
    for (const auto& c : clauses) {
        std::cout << "  ";
        for (int lit : c) std::cout << lit << " ";
        std::cout << "\n";
    }

    auto result = GoldenExactGates::GoldenExactSolver::solve(clauses, num_vars);
    std::cout << "\nSat: " << result.satisfiable << "\n";
    if (result.satisfiable) {
        std::cout << "Assignment: ";
        for (bool v : result.assignment) std::cout << v << " ";
        std::cout << "\n";
    }

    return 0;
}
