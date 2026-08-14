#include "src/np/complete_dpll.h"
#include <iostream>
#include <random>
#include <set>
#include <chrono>

int main() {
    std::cout << "=== COMPLETE DPLL (Guaranteed Correct) ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    int total_tests = 0;
    int total_pass = 0;
    
    for (int num_vertices : {5, 10, 15, 20}) {
        for (int test = 0; test < 3; test++) {
            // Generate guaranteed 3-colorable
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
            
            auto result = CompleteDPLL::CompleteDPLLSolver::solve(clauses, num_vars);
            
            total_tests++;
            if (result.satisfiable) {
                // Verify assignment
                bool valid = true;
                for (const auto& clause : clauses) {
                    bool sat = false;
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        bool val = result.assignment[var];
                        if ((lit > 0 && val) || (lit < 0 && !val)) {
                            sat = true;
                            break;
                        }
                    }
                    if (!sat) { valid = false; break; }
                }
                
                if (valid) {
                    total_pass++;
                    std::cout << "  " << num_vertices << " vertices: PASS (steps=" 
                             << result.steps << ")\n";
                } else {
                    std::cout << "  " << num_vertices << " vertices: INVALID\n";
                }
            } else {
                std::cout << "  " << num_vertices << " vertices: UNSAT (BUG!)\n";
            }
        }
    }
    
    std::cout << "\nTOTAL: " << total_pass << "/" << total_tests << "\n";
    
    return 0;
}
