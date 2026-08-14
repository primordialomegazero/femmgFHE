#include "src/np/golden_complete_optimized.h"
#include <iostream>
#include <chrono>
#include <random>

int main() {
    std::cout << "=== SOLVER PROFILE ===\n\n";
    
    // Test sa iba't ibang sizes
    for (int num_vars : {10, 50, 100, 500, 1000, 5000}) {
        int num_clauses = num_vars * 3;
        
        std::vector<std::vector<int>> clauses;
        std::mt19937 gen(42);
        
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
        auto result = GoldenCompleteOptimized::GoldenCompleteSolver::solve(clauses, num_vars);
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << num_vars << " vars: " << (result.satisfiable ? "SAT" : "UNSAT")
                 << " (steps=" << result.steps 
                 << ", backtracks=" << result.backtracks
                 << ", time=" << ms << "ms)\n";
        
        if (ms > 10000) {
            std::cout << "  TIMEOUT - stopping\n";
            break;
        }
    }
    
    return 0;
}
