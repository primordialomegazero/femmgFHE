#include "src/np/backtracking_solver.h"
#include <iostream>
#include <random>
#include <chrono>
#include <set>

// Generate guaranteed 3-colorable graph
std::vector<std::pair<int,int>> generate_3colorable_graph(int num_vertices, std::mt19937& gen) {
    std::vector<std::pair<int,int>> edges;
    std::set<std::pair<int,int>> edge_set;
    
    // Assign colors to guarantee 3-colorability
    std::vector<int> colors(num_vertices);
    for (int i = 0; i < num_vertices; i++) {
        colors[i] = gen() % 3;
    }
    
    // Add edges between different colored vertices
    for (int i = 0; i < num_vertices; i++) {
        for (int j = i+1; j < num_vertices; j++) {
            if (colors[i] != colors[j] && gen() % 100 < 30) {
                edges.push_back({i, j});
                edge_set.insert({i, j});
            }
        }
    }
    
    while (edges.size() < num_vertices) {
        int u = gen() % num_vertices;
        int v = gen() % num_vertices;
        if (u != v && colors[u] != colors[v] && edge_set.find({u, v}) == edge_set.end()) {
            edges.push_back({u, v});
            edge_set.insert({u, v});
        }
    }
    
    return edges;
}

int main() {
    std::cout << "=== DEBUG DPLL - 15 VERTICES ===\n\n";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Test with seed that fails
    for (int seed = 1; seed <= 10; seed++) {
        std::mt19937 gen2(seed);
        auto edges = generate_3colorable_graph(15, gen2);
        
        std::vector<std::vector<int>> clauses;
        int num_vars = 15 * 3;
        
        for (int v = 0; v < 15; v++) {
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
        
        std::cout << "Seed " << seed << ": ";
        std::cout << "edges=" << edges.size() << ", ";
        
        auto result = BacktrackingSolver::DPLLSolver::solve(clauses, num_vars);
        
        if (result.satisfiable) {
            std::cout << "PASS (steps=" << result.steps << ")\n";
        } else {
            std::cout << "FAIL (steps=" << result.steps << ", backtracks=" 
                     << result.backtracks << ")\n";
            
            // Check if graph is actually 3-colorable
            std::cout << "  Edges: ";
            for (auto& e : edges) {
                std::cout << "(" << e.first << "," << e.second << ") ";
            }
            std::cout << "\n";
        }
    }
    
    return 0;
}
