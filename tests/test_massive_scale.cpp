#include "src/np/golden_ultimate_solver.h"
#include <iostream>
#include <random>
#include <chrono>
#include <set>

int main() {
    std::cout << "=== MASSIVE SCALE TEST (10,000+) ===\n\n";
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int num_vertices : {2000, 5000, 10000}) {
        int num_vars = num_vertices * 3;
        
        // Generate sparse 3-colorable graph
        std::vector<int> colors(num_vertices);
        std::vector<std::pair<int,int>> edges;
        
        for (int i = 0; i < num_vertices; i++) {
            colors[i] = gen() % 3;
        }
        
        // Sparse edges (just enough to be non-trivial)
        for (int i = 0; i < num_vertices; i++) {
            int j = (i + 1) % num_vertices;
            if (colors[i] != colors[j]) {
                edges.push_back({i, j});
            }
        }
        
        // Add some random edges
        for (int e = 0; e < num_vertices * 2; e++) {
            int u = gen() % num_vertices;
            int v = gen() % num_vertices;
            if (u != v && colors[u] != colors[v]) {
                edges.push_back({u, v});
            }
        }
        
        std::vector<std::vector<int>> clauses;
        
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
        
        std::cout << "Testing " << num_vertices << " vertices (" << num_vars 
                  << " vars, " << clauses.size() << " clauses)...\n";
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_vars);
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "  Result: " << (result.satisfiable ? "SAT" : "UNSAT") 
                  << " (time=" << ms << "ms, steps=" << result.steps << ")\n";
    }
    
    return 0;
}
