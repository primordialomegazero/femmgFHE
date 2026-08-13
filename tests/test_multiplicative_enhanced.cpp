#include "src/np/golden_multiplicative_enhanced.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== GOLDEN MULTIPLICATIVE ENHANCED SOLVER ===\n\n";
    
    int total_tests = 0;
    int total_pass = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // TEST 1: Graph 3-coloring
    std::cout << "--- Graph 3-Coloring ---\n";
    for (int test = 0; test < 5; test++) {
        int num_vertices = 10 + test * 5;
        int num_edges = num_vertices * 2;
        
        std::vector<std::pair<int,int>> edges;
        for (int e = 0; e < num_edges; e++) {
            int u = gen() % num_vertices;
            int v = gen() % num_vertices;
            if (u != v) edges.push_back({u, v});
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
        
        auto result = GoldenMultiplicativeEnhanced::GoldenMultiplicativeSolver::solve(
            clauses, num_vars, 30, 2000);
        
        total_tests++;
        if (result.satisfiable) {
            total_pass++;
            std::cout << "  Graph " << num_vertices << " vertices: PASS (steps=" 
                     << result.steps << ")\n";
        } else {
            std::cout << "  Graph " << num_vertices << " vertices: FAIL\n";
        }
    }
    
    // TEST 2: N-Queens
    std::cout << "\n--- N-Queens ---\n";
    for (int n = 4; n <= 10; n += 2) {
        int num_vars = n * n;
        std::vector<std::vector<int>> clauses;
        
        for (int i = 0; i < n; i++) {
            std::vector<int> clause;
            for (int j = 0; j < n; j++) {
                clause.push_back(i*n + j + 1);
            }
            clauses.push_back(clause);
        }
        
        for (int i = 0; i < n; i++) {
            for (int j1 = 0; j1 < n; j1++) {
                for (int j2 = j1+1; j2 < n; j2++) {
                    clauses.push_back({-(i*n+j1+1), -(i*n+j2+1)});
                }
            }
        }
        
        for (int j = 0; j < n; j++) {
            for (int i1 = 0; i1 < n; i1++) {
                for (int i2 = i1+1; i2 < n; i2++) {
                    clauses.push_back({-(i1*n+j+1), -(i2*n+j+1)});
                }
            }
        }
        
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
        
        auto result = GoldenMultiplicativeEnhanced::GoldenMultiplicativeSolver::solve(
            clauses, num_vars, 30, 2000);
        
        total_tests++;
        if (result.satisfiable) {
            total_pass++;
            std::cout << "  " << n << "-Queens: PASS (steps=" 
                     << result.steps << ")\n";
        } else {
            std::cout << "  " << n << "-Queens: FAIL\n";
        }
    }
    
    std::cout << "\n============================\n";
    std::cout << "TOTAL: " << total_pass << "/" << total_tests << "\n";
    if (total_pass == total_tests) {
        std::cout << "ALL PASS ✅\n";
    } else {
        std::cout << "SOME FAIL ❌\n";
    }
    
    return 0;
}
