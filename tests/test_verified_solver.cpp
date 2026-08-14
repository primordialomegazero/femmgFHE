#include "src/np/golden_verified_solver.h"
#include <iostream>
#include <random>
#include <set>

int main() {
    std::cout << "=== VERIFIED GOLDEN SOLVER ===\n\n";
    std::random_device rd;
    std::mt19937 gen(rd());
    
    int total_tests = 0;
    int total_pass = 0;
    
    for (int num_vertices : {10, 20, 50, 100}) {
        for (int test = 0; test < 5; test++) {
            // Generate guaranteed 3-colorable
            std::vector<int> colors(num_vertices);
            std::vector<std::pair<int,int>> edges;
            std::set<std::pair<int,int>> edge_set;
            
            for (int i = 0; i < num_vertices; i++) colors[i] = gen() % 3;
            
            for (int i = 0; i < num_vertices; i++) {
                for (int j = i+1; j < num_vertices; j++) {
                    if (colors[i] != colors[j] && gen() % 100 < 40) {
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
            
            auto result = GoldenVerified::GoldenVerifiedSolver::solve(clauses, num_vars);
            
            total_tests++;
            if (result.satisfiable) {
                // Verify manually
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
                    std::cout << "  " << num_vertices << " vertices: PASS (verified)\n";
                } else {
                    std::cout << "  " << num_vertices << " vertices: FAIL (invalid assignment)\n";
                }
            } else {
                std::cout << "  " << num_vertices << " vertices: FAIL (UNSAT)\n";
            }
        }
    }
    
    std::cout << "\nTOTAL: " << total_pass << "/" << total_tests << "\n";
    
    return 0;
}
