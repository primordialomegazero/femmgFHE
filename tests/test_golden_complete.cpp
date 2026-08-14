#include "src/np/golden_complete_optimized.h"
#include "src/np/complete_dpll.h"
#include <iostream>
#include <random>
#include <set>
#include <chrono>

int main() {
    std::cout << "=== GOLDEN COMPLETE vs BASELINE DPLL ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    int total_tests = 0;
    int golden_pass = 0;
    int baseline_pass = 0;
    
    for (int num_vertices : {5, 10, 15, 20, 25}) {
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
            
            total_tests++;
            
            // Test golden optimized
            auto golden_result = GoldenCompleteOptimized::GoldenCompleteSolver::solve(clauses, num_vars);
            if (golden_result.satisfiable) golden_pass++;
            
            // Test baseline
            auto baseline_result = CompleteDPLL::CompleteDPLLSolver::solve(clauses, num_vars);
            if (baseline_result.satisfiable) baseline_pass++;
            
            std::cout << "  " << num_vertices << " vertices (test " << test+1 << "): ";
            std::cout << "Golden=" << (golden_result.satisfiable ? "SAT" : "UNSAT")
                     << "(steps=" << golden_result.steps << ") ";
            std::cout << "Baseline=" << (baseline_result.satisfiable ? "SAT" : "UNSAT")
                     << "(steps=" << baseline_result.steps << ")\n";
        }
    }
    
    std::cout << "\nGolden: " << golden_pass << "/" << total_tests << "\n";
    std::cout << "Baseline: " << baseline_pass << "/" << total_tests << "\n";
    
    return 0;
}
