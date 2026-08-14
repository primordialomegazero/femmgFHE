#include "src/np/golden_noise_collapse.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== GOLDEN NOISE COLLAPSE ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    int total_tests = 0;
    int total_pass = 0;
    
    for (int num_vars : {10, 50, 100, 200, 500, 1000}) {
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
            
            auto result = GoldenNoiseCollapse::GoldenNoiseSolver::solve(clauses, num_vars);
            
            total_tests++;
            if (result.satisfiable) {
                total_pass++;
            }
        }
        
        std::cout << "  " << num_vars << " vars: " << total_pass << "/" << total_tests << " SAT\n";
        total_tests = 0;
        total_pass = 0;
    }
    
    return 0;
}
