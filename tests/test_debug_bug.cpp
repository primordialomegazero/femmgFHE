#include "src/np/golden_ultimate_solver.h"
#include <iostream>
#include <random>
#include <set>

int main() {
    std::cout << "=== DEBUG FALSE POSITIVE ===\n\n";
    std::random_device rd;
    std::mt19937 gen(12345);  // Fixed seed para reproducible
    
    // Generate small 3-colorable graph na may bug
    int num_vertices = 10;
    std::vector<int> known_colors(num_vertices);
    std::vector<std::pair<int,int>> edges;
    std::set<std::pair<int,int>> edge_set;
    
    for (int i = 0; i < num_vertices; i++) {
        known_colors[i] = gen() % 3;
    }
    
    for (int i = 0; i < num_vertices; i++) {
        for (int j = i+1; j < num_vertices; j++) {
            if (known_colors[i] != known_colors[j] && gen() % 100 < 40) {
                edges.push_back({i, j});
                edge_set.insert({i, j});
            }
        }
    }
    
    // Build SAT
    std::vector<std::vector<int>> clauses;
    int num_vars = num_vertices * 3;
    
    for (int v = 0; v < num_vertices; v++) {
        int r = v*3, g = v*3+1, b = v*3+2;
        clauses.push_back({r+1, g+1, b+1});
        clauses.push_back({-(r+1), -(g+1)});
        clauses.push_back({-(r+1), -(b+1)});
        clauses.push_back({-(g+1), -(b+1)});
    }
    
    for (auto& edge : edges) {
        int u = edge.first, v = edge.second;
        for (int c = 0; c < 3; c++) {
            clauses.push_back({-(u*3+c+1), -(v*3+c+1)});
        }
    }
    
    // Print known correct coloring
    std::cout << "Known colors: ";
    for (int c : known_colors) std::cout << c << " ";
    std::cout << "\n\n";
    
    // Solve
    auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_vars);
    
    std::cout << "Solver: " << (result.satisfiable ? "SAT" : "UNSAT") << "\n";
    std::cout << "Steps: " << result.steps << ", Backtracks: " << result.backtracks << "\n";
    
    // Extract and print solver's coloring
    if (result.satisfiable) {
        std::cout << "Solver colors: ";
        for (int v = 0; v < num_vertices; v++) {
            for (int c = 0; c < 3; c++) {
                if (result.assignment[v*3 + c]) {
                    std::cout << c << " ";
                    break;
                }
            }
        }
        std::cout << "\n\n";
        
        // Check each edge
        std::cout << "Edge violations:\n";
        for (auto& edge : edges) {
            int color_u = -1, color_v = -1;
            for (int c = 0; c < 3; c++) {
                if (result.assignment[edge.first*3 + c]) color_u = c;
                if (result.assignment[edge.second*3 + c]) color_v = c;
            }
            if (color_u == color_v) {
                std::cout << "  VIOLATION: Edge (" << edge.first << "," << edge.second 
                         << ") both color " << color_u << "\n";
            }
        }
        
        // Check if any vertex has multiple colors
        std::cout << "\nMultiple color assignments:\n";
        for (int v = 0; v < num_vertices; v++) {
            int count = 0;
            for (int c = 0; c < 3; c++) {
                if (result.assignment[v*3 + c]) count++;
            }
            if (count != 1) {
                std::cout << "  Vertex " << v << " has " << count << " colors\n";
            }
        }
    }
    
    return 0;
}
