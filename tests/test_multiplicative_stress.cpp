#include "src/np/golden_multiplicative.h"
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>

int main() {
    std::cout << "=== GOLDEN MULTIPLICATIVE SOLVER STRESS ===\n\n";
    
    int total_tests = 0;
    int total_pass = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // ==========================================
    // TEST 1: Graph Coloring (3-coloring)
    // ==========================================
    std::cout << "--- Graph 3-Coloring ---\n";
    for (int test = 0; test < 5; test++) {
        int num_vertices = 10 + test * 5;  // 10, 15, 20, 25, 30
        int num_edges = num_vertices * 2;   // Sparse graph
        
        // Generate random graph
        std::vector<std::pair<int,int>> edges;
        for (int e = 0; e < num_edges; e++) {
            int u = gen() % num_vertices;
            int v = gen() % num_vertices;
            if (u != v) edges.push_back({u, v});
        }
        
        // Convert to 3-coloring SAT
        // Each vertex has 3 boolean vars (r,g,b) - exactly one must be true
        std::vector<std::vector<int>> clauses;
        int num_vars = num_vertices * 3;
        
        for (int v = 0; v < num_vertices; v++) {
            int r = v*3, g = v*3+1, b = v*3+2;
            // At least one color
            clauses.push_back({r+1, g+1, b+1});
            // At most one color
            clauses.push_back({-(r+1), -(g+1)});
            clauses.push_back({-(r+1), -(b+1)});
            clauses.push_back({-(g+1), -(b+1)});
        }
        
        // Edge constraints: different colors
        for (auto& edge : edges) {
            int u = edge.first, v = edge.second;
            for (int c = 0; c < 3; c++) {
                clauses.push_back({-(u*3+c+1), -(v*3+c+1)});
            }
        }
        
        auto result = GoldenMultiplicative::GoldenMultiplicativeSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) {
            total_pass++;
            std::cout << "  Graph " << num_vertices << " vertices: sat=1 "
                     << "score=" << result.golden_score << " "
                     << "steps=" << result.steps << "\n";
        } else {
            std::cout << "  Graph " << num_vertices << " vertices: sat=0 (FAIL)\n";
        }
    }
    
    // ==========================================
    // TEST 2: N-Queens as SAT
    // ==========================================
    std::cout << "\n--- N-Queens ---\n";
    for (int n = 4; n <= 12; n += 2) {
        int num_vars = n * n;
        std::vector<std::vector<int>> clauses;
        
        // At least one queen per row
        for (int i = 0; i < n; i++) {
            std::vector<int> clause;
            for (int j = 0; j < n; j++) {
                clause.push_back(i*n + j + 1);
            }
            clauses.push_back(clause);
        }
        
        // At most one queen per row
        for (int i = 0; i < n; i++) {
            for (int j1 = 0; j1 < n; j1++) {
                for (int j2 = j1+1; j2 < n; j2++) {
                    clauses.push_back({-(i*n+j1+1), -(i*n+j2+1)});
                }
            }
        }
        
        // At most one queen per column
        for (int j = 0; j < n; j++) {
            for (int i1 = 0; i1 < n; i1++) {
                for (int i2 = i1+1; i2 < n; i2++) {
                    clauses.push_back({-(i1*n+j+1), -(i2*n+j+1)});
                }
            }
        }
        
        // At most one queen per diagonal
        for (int i1 = 0; i1 < n; i1++) {
            for (int j1 = 0; j1 < n; j1++) {
                for (int i2 = i1+1; i2 < n; i2++) {
                    int dj = i2 - i1;
                    if (j1 + dj < n) {
                        clauses.push_back({-(i1*n+j1+1), -(i2*n+(j1+dj)+1)});
                    }
                    if (j1 - dj >= 0) {
                        clauses.push_back({-(i1*n+j1+1), -(i2*n+(j1-dj)+1)});
                    }
                }
            }
        }
        
        auto result = GoldenMultiplicative::GoldenMultiplicativeSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) {
            total_pass++;
            std::cout << "  " << n << "-Queens: sat=1 "
                     << "steps=" << result.steps << "\n";
        } else {
            std::cout << "  " << n << "-Queens: sat=0 (FAIL)\n";
        }
    }
    
    // ==========================================
    // TEST 3: Random 3-SAT Scaling
    // ==========================================
    std::cout << "\n--- Random 3-SAT Scaling ---\n";
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (int num_vars : {50, 100, 200, 400, 800, 1600}) {
        int num_clauses = num_vars * 3;  // Below phase transition
        std::vector<std::vector<int>> clauses;
        
        for (int c = 0; c < num_clauses; c++) {
            std::vector<int> clause;
            for (int k = 0; k < 3; k++) {
                int var = gen() % num_vars + 1;
                if (dis(gen) < 0.5) var = -var;
                clause.push_back(var);
            }
            clauses.push_back(clause);
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = GoldenMultiplicative::GoldenMultiplicativeSolver::solve(clauses, num_vars);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        total_tests++;
        if (result.satisfiable) {
            total_pass++;
            std::cout << "  " << num_vars << " vars: sat=1 "
                     << "time=" << duration.count() / 1000.0 << "ms "
                     << "steps=" << result.steps << "\n";
        }
    }
    
    std::cout << "\n============================\n";
    std::cout << "TOTAL: " << total_pass << "/" << total_tests;
    if (total_pass == total_tests) {
        std::cout << "\nALL PASS ✅\n";
    } else {
        std::cout << "\nSOME FAIL ❌\n";
    }
    
    return 0;
}
