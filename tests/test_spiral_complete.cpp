#include "src/np/spiral_complete_sat.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== COMPLETE SPIRAL GOLDEN SAT ===\n\n";
    std::random_device rd;
    std::mt19937 gen(rd());
    
    int total_tests = 0;
    int total_pass = 0;
    
    // Test lahat ng sizes
    for (int num_vars : {10, 20, 50, 100}) {
        int num_clauses = num_vars * 3;
        
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
            
            auto result = SpiralCompleteSAT::SpiralCompleteSolver::solve(clauses, num_vars);
            
            total_tests++;
            if (result.satisfiable) {
                total_pass++;
                std::cout << "  " << num_vars << " vars (test " << test+1 
                         << "): PASS (time=" << result.time_ms << "ms)\n";
            } else {
                std::cout << "  " << num_vars << " vars (test " << test+1 
                         << "): FAIL\n";
            }
        }
    }
    
    std::cout << "\nTOTAL: " << total_pass << "/" << total_tests 
              << " (" << (total_pass * 100.0 / total_tests) << "%)\n";
    
    return 0;
}
