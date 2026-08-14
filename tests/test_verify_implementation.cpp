#include "src/np/golden_ultimate_solver.h"
#include <iostream>
#include <random>
#include <chrono>
#include <set>

// Independent verification ng assignment
bool verify_assignment(const std::vector<std::vector<int>>& clauses,
                       const std::vector<bool>& assignment) {
    for (const auto& clause : clauses) {
        bool sat = false;
        for (int lit : clause) {
            int var = abs(lit) - 1;
            if (var < assignment.size()) {
                bool val = assignment[var];
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    sat = true;
                    break;
                }
            }
        }
        if (!sat) {
            std::cout << "  UNSATISFIED CLAUSE: ";
            for (int lit : clause) std::cout << lit << " ";
            std::cout << "\n";
            return false;
        }
    }
    return true;
}

// Independent 3-coloring verification
bool verify_3coloring(const std::vector<std::pair<int,int>>& edges,
                      const std::vector<bool>& assignment) {
    // Extract colors from boolean assignment
    int num_vertices = edges.size() > 0 ? 
        std::max(edges.back().first, edges.back().second) + 1 : 0;
    
    std::vector<int> colors(num_vertices, -1);
    for (int v = 0; v < num_vertices; v++) {
        for (int c = 0; c < 3; c++) {
            if (assignment[v*3 + c]) {
                if (colors[v] != -1) {
                    std::cout << "  VERTEX " << v << " HAS MULTIPLE COLORS!\n";
                    return false;
                }
                colors[v] = c;
            }
        }
        if (colors[v] == -1) {
            std::cout << "  VERTEX " << v << " HAS NO COLOR!\n";
            return false;
        }
    }
    
    // Check edges
    for (auto& edge : edges) {
        if (colors[edge.first] == colors[edge.second]) {
            std::cout << "  EDGE (" << edge.first << "," << edge.second 
                     << ") SAME COLOR!\n";
            return false;
        }
    }
    return true;
}

int main() {
    std::cout << "=== IMPLEMENTATION VERIFICATION ===\n\n";
    std::random_device rd;
    std::mt19937 gen(rd());
    
    int total_verified = 0;
    int total_failed = 0;
    
    for (int num_vertices : {10, 20, 50, 100, 200}) {
        for (int test = 0; test < 5; test++) {
            // Generate known 3-colorable graph
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
            
            // Solve
            auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_vars);
            
            if (!result.satisfiable) {
                std::cout << "FAIL: " << num_vertices << " vertices reported UNSAT\n";
                total_failed++;
                continue;
            }
            
            // Verify SAT assignment
            bool sat_verified = verify_assignment(clauses, result.assignment);
            bool coloring_verified = verify_3coloring(edges, result.assignment);
            
            if (sat_verified && coloring_verified) {
                total_verified++;
            } else {
                std::cout << "FAIL: " << num_vertices << " vertices verification failed\n";
                total_failed++;
            }
        }
    }
    
    std::cout << "\nVerified: " << total_verified << "/" << (total_verified + total_failed) << "\n";
    if (total_failed == 0) {
        std::cout << "ALL VERIFIED ✅\n";
    } else {
        std::cout << "SOME FAILED ❌\n";
    }
    
    return 0;
}
