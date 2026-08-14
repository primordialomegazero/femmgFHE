#include "src/np/golden_verified_solver.h"
#include <iostream>
#include <random>
#include <set>
#include <chrono>

int main() {
    std::cout << "=== DEBUG VERIFICATION ISSUE ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    for (int num_vertices = 20; num_vertices <= 50; num_vertices += 10) {
        std::cout << "--- " << num_vertices << " vertices ---\n";
        
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
            
            // Build SAT formula
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
            
            // Verify na ang known coloring ay nagse-satisfy sa SAT
            bool known_sat = true;
            for (const auto& clause : clauses) {
                bool sat = false;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    int vertex = var / 3;
                    int color = var % 3;
                    bool val = (known_colors[vertex] == color);
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        sat = true;
                        break;
                    }
                }
                if (!sat) {
                    known_sat = false;
                    std::cout << "  Known coloring FAILS clause: ";
                    for (int lit : clause) std::cout << lit << " ";
                    std::cout << "\n";
                    break;
                }
            }
            
            std::cout << "  Test " << test+1 << ": edges=" << edges.size() 
                     << ", known_sat=" << (known_sat ? "YES" : "NO");
            
            // Solve
            auto result = GoldenVerified::GoldenVerifiedSolver::solve(clauses, num_vars);
            std::cout << ", solver=" << (result.satisfiable ? "SAT" : "UNSAT");
            
            if (known_sat && !result.satisfiable) {
                std::cout << " *** BUG: Should be SAT! ***";
            }
            std::cout << "\n";
        }
    }
    
    return 0;
}
