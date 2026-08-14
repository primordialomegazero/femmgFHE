#include "src/np/golden_verified_solver.h"
#include <iostream>
#include <random>
#include <set>

int main() {
    std::cout << "=== DEBUG BACKTRACKING ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    // Small case na nag-fail
    int num_vertices = 20;
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
    
    std::cout << "Known colors: ";
    for (int c : known_colors) std::cout << c << " ";
    std::cout << "\n";
    std::cout << "Edges: " << edges.size() << "\n\n";
    
    // Solve with debug
    auto result = GoldenVerified::GoldenVerifiedSolver::solve(clauses, num_vars);
    
    std::cout << "Result: " << (result.satisfiable ? "SAT" : "UNSAT") << "\n";
    std::cout << "Steps: " << result.steps << ", Backtracks: " << result.backtracks << "\n";
    std::cout << "Conflicts: " << result.conflicts << "\n";
    
    // Manual check: can we construct valid assignment from known colors?
    std::vector<bool> test_assignment(num_vars, false);
    for (int v = 0; v < num_vertices; v++) {
        test_assignment[v*3 + known_colors[v]] = true;
    }
    
    // Verify known assignment
    bool valid = true;
    for (const auto& clause : clauses) {
        bool sat = false;
        for (int lit : clause) {
            int var = abs(lit) - 1;
            bool val = test_assignment[var];
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                sat = true;
                break;
            }
        }
        if (!sat) {
            valid = false;
            std::cout << "Known assignment violates clause: ";
            for (int lit : clause) std::cout << lit << " ";
            std::cout << "\n";
            break;
        }
    }
    
    std::cout << "Known assignment valid: " << (valid ? "YES" : "NO") << "\n";
    
    return 0;
}
