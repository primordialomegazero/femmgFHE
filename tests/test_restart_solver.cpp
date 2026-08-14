#include "src/np/golden_restart_solver.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== GOLDEN RESTART SOLVER ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    int total_tests = 0;
    int total_pass = 0;
    
    for (int num_vars : {10, 50, 100, 200, 500}) {
        int num_clauses = num_vars * 3;
        
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
            auto result = GoldenRestart::GoldenRestartSolver::solve(clauses, num_vars);
            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            
            total_tests++;
            if (result.satisfiable) {
                total_pass++;
                std::cout << "  " << num_vars << " vars: SAT (restarts=" 
                         << result.restarts << ", time=" << ms << "ms)\n";
            }
        }
    }
    
    std::cout << "\nTotal: " << total_pass << "/" << total_tests << "\n";
    
    return 0;
}
