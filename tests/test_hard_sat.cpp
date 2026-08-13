#include "src/np/golden_cdcl_solver.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== HARD SAT INSTANCES TEST ===\n\n";
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Test phase transition region (clause/variable ratio ≈ 4.26)
    std::cout << "--- Phase Transition (ratio=4.26) ---\n";
    for (int num_vars : {50, 100, 150, 200, 250, 300}) {
        int num_clauses = (int)(num_vars * 4.26);
        
        for (int test = 0; test < 3; test++) {
            std::vector<std::vector<int>> clauses;
            for (int c = 0; c < num_clauses; c++) {
                std::vector<int> clause;
                for (int k = 0; k < 3; k++) {
                    int var = gen() % num_vars + 1;
                    if (gen() % 2 == 0) var = -var;
                    clause.push_back(var);
                }
                clauses.push_back(clause);
            }
            
            auto start = std::chrono::high_resolution_clock::now();
            auto result = GoldenCDCL::GoldenCDCLSolver::solve(clauses, num_vars);
            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            
            std::cout << "  " << num_vars << " vars, " << num_clauses << " clauses (test " 
                     << test+1 << "): " << (result.satisfiable ? "SAT" : "UNSAT")
                     << " (steps=" << result.steps 
                     << ", conflicts=" << result.conflicts
                     << ", time=" << ms << "ms)\n";
            
            if (ms > 5000) {
                std::cout << "  *** TIMEOUT: >5 seconds! ***\n";
                break;
            }
        }
    }
    
    // Test unsatisfiable instances
    std::cout << "\n--- Unsatisfiable Instances ---\n";
    for (int n : {10, 20, 30, 40, 50}) {
        // Pigeonhole principle: n+1 pigeons, n holes (classic UNSAT)
        int num_vars = (n+1) * n;
        std::vector<std::vector<int>> clauses;
        
        // Each pigeon in at least one hole
        for (int p = 0; p <= n; p++) {
            std::vector<int> clause;
            for (int h = 0; h < n; h++) {
                clause.push_back(p*n + h + 1);
            }
            clauses.push_back(clause);
        }
        
        // No two pigeons in same hole
        for (int h = 0; h < n; h++) {
            for (int p1 = 0; p1 <= n; p1++) {
                for (int p2 = p1+1; p2 <= n; p2++) {
                    clauses.push_back({-(p1*n + h + 1), -(p2*n + h + 1)});
                }
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = GoldenCDCL::GoldenCDCLSolver::solve(clauses, num_vars);
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "  Pigeonhole " << n+1 << "/" << n << ": " 
                 << (result.satisfiable ? "SAT (BUG!)" : "UNSAT (correct)")
                 << " (time=" << ms << "ms)\n";
    }
    
    return 0;
}
