#include "src/np/golden_ultimate_solver.h"
#include <iostream>
#include <random>
#include <chrono>
#include <set>

std::vector<std::pair<int,int>> generate_3colorable_graph(int num_vertices, std::mt19937& gen) {
    std::vector<std::pair<int,int>> edges;
    std::set<std::pair<int,int>> edge_set;
    
    std::vector<int> colors(num_vertices);
    for (int i = 0; i < num_vertices; i++) {
        colors[i] = gen() % 3;
    }
    
    for (int i = 0; i < num_vertices; i++) {
        for (int j = i+1; j < num_vertices; j++) {
            if (colors[i] != colors[j] && gen() % 100 < 40) {
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
    std::cout << "=== GOLDEN MASSIVE SCALE TEST ===\n\n";
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Test sa malalaking instances
    for (int num_vertices : {300, 400, 500, 750, 1000}) {
        int num_vars = num_vertices * 3;
        auto edges = generate_3colorable_graph(num_vertices, gen);
        
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
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_vars);
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << num_vertices << " vertices (" << num_vars << " vars): ";
        if (result.satisfiable) {
            std::cout << "PASS (time=" << ms << "ms, steps=" << result.steps << ")\n";
        } else {
            std::cout << "FAIL\n";
        }
    }
    
    return 0;
}
