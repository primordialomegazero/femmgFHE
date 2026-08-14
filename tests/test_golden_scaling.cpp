#include "src/np/golden_exact_solver.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== GOLDEN EXACT SCALING TEST ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    // Test sa mas malalaking instances
    std::cout << "--- Easy (ratio=3) ---\n";
    for (int num_vars : {40, 50, 75, 100}) {
        int num_clauses = num_vars * 3;
        
        auto start_total = std::chrono::high_resolution_clock::now();
        int sat_count = 0;
        
        for (int test = 0; test < 5; test++) {
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
            
            auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
            if (result.satisfiable) sat_count++;
        }
        
        auto end_total = std::chrono::high_resolution_clock::now();
        auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_total - start_total).count();
        
        std::cout << "  " << num_vars << " vars: " << sat_count << "/5 SAT, "
                 << total_ms << "ms total\n";
    }
    
    // Test sa hard instances (phase transition)
    std::cout << "\n--- Hard (ratio=4.26) ---\n";
    for (int num_vars : {50, 100, 150}) {
        int num_clauses = (int)(num_vars * 4.26);
        
        auto start_total = std::chrono::high_resolution_clock::now();
        int sat_count = 0;
        int total_steps = 0;
        
        for (int test = 0; test < 5; test++) {
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
            
            auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
            if (result.satisfiable) sat_count++;
            total_steps += result.steps;
        }
        
        auto end_total = std::chrono::high_resolution_clock::now();
        auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_total - start_total).count();
        
        std::cout << "  " << num_vars << " vars: " << sat_count << "/5 SAT, "
                 << total_ms << "ms, avg steps=" << (total_steps/5) << "\n";
    }
    
    return 0;
}
