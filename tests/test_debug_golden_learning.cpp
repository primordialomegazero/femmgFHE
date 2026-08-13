#include "src/np/golden_learning_solver.h"
#include <iostream>
#include <random>
#include <chrono>
#include <set>

// Generate guaranteed 3-colorable graph WITH known coloring
std::vector<std::pair<int,int>> generate_3colorable_graph(int num_vertices, 
                                                          std::mt19937& gen,
                                                          std::vector<int>& known_colors) {
    std::vector<std::pair<int,int>> edges;
    std::set<std::pair<int,int>> edge_set;
    
    known_colors.resize(num_vertices);
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
    
    while (edges.size() < num_vertices) {
        int u = gen() % num_vertices;
        int v = gen() % num_vertices;
        if (u != v && known_colors[u] != known_colors[v] && 
            edge_set.find({u, v}) == edge_set.end()) {
            edges.push_back({u, v});
            edge_set.insert({u, v});
        }
    }
    
    return edges;
}

// Manual verification ng assignment
bool verify_assignment(const std::vector<std::vector<int>>& clauses,
                       const std::vector<bool>& assignment) {
    for (const auto& clause : clauses) {
        bool sat = false;
        for (int lit : clause) {
            int var = abs(lit) - 1;
            bool val = assignment[var];
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                sat = true;
                break;
            }
        }
        if (!sat) return false;
    }
    return true;
}

int main() {
    std::cout << "=== DEEP DEBUG GOLDEN LEARNING ===\n\n";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Test specific failing case: 20 vertices
    for (int seed = 1; seed <= 5; seed++) {
        std::mt19937 gen2(seed);
        std::vector<int> known_colors;
        auto edges = generate_3colorable_graph(20, gen2, known_colors);
        
        std::cout << "\n--- Seed " << seed << " (20 vertices, " << edges.size() << " edges) ---\n";
        
        // Build clauses
        std::vector<std::vector<int>> clauses;
        int num_vars = 20 * 3;
        
        for (int v = 0; v < 20; v++) {
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
        
        // Solve
        auto result = GoldenLearning::GoldenLearningSolver::solve(clauses, num_vars);
        
        std::cout << "Solver: " << (result.satisfiable ? "PASS" : "FAIL") << "\n";
        std::cout << "Steps: " << result.steps << ", Backtracks: " << result.backtracks << "\n";
        
        if (!result.satisfiable) {
            std::cout << "  Known coloring exists: ";
            for (int i = 0; i < 10; i++) {
                std::cout << known_colors[i] << " ";
            }
            std::cout << "...\n";
            
            // Check if known coloring satisfies the SAT
            std::vector<bool> known_assignment(num_vars, false);
            for (int v = 0; v < 20; v++) {
                known_assignment[v*3 + known_colors[v]] = true;
            }
            
            bool known_sat = verify_assignment(clauses, known_assignment);
            std::cout << "  Known coloring satisfies SAT: " << (known_sat ? "YES" : "NO") << "\n";
            
            if (known_sat) {
                std::cout << "  *** BUG: Solver FAILED pero may valid assignment! ***\n";
            }
            
            // Print solver's partial assignment
            std::cout << "  Solver assignment (first 15 vars): ";
            for (int i = 0; i < 15 && i < result.assignment.size(); i++) {
                std::cout << result.assignment[i] << " ";
            }
            std::cout << "\n";
        }
    }
    
    return 0;
}
