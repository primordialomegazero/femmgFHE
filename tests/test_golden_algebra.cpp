#include "src/np/golden_algebra_sat.h"
#include <iostream>
#include <random>

int main() {
    std::cout << "=== GOLDEN ALGEBRA SAT ===\n\n";
    
    // Test basic operations
    std::cout << "Golden OR truth table:\n";
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            double ga = GoldenAlgebra::GoldenAlgebraSolver::to_golden(a);
            double gb = GoldenAlgebra::GoldenAlgebraSolver::to_golden(b);
            double result = GoldenAlgebra::GoldenAlgebraSolver::golden_or(ga, gb);
            std::cout << "  " << a << " OR " << b << " = " 
                     << (result > 0 ? "TRUE" : "FALSE") 
                     << " (value=" << result << ")\n";
        }
    }
    
    std::cout << "\nGolden NOT:\n";
    std::cout << "  NOT TRUE = " << GoldenAlgebra::GoldenAlgebraSolver::golden_not(GoldenAlgebra::PHI) << "\n";
    std::cout << "  NOT FALSE = " << GoldenAlgebra::GoldenAlgebraSolver::golden_not(GoldenAlgebra::PSI) << "\n";
    
    // Test SAT solving
    std::cout << "\n--- Simple SAT ---\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    int total_tests = 0;
    int total_pass = 0;
    
    for (int num_vars : {5, 10, 20}) {
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
            
            auto result = GoldenAlgebra::GoldenAlgebraSolver::solve(clauses, num_vars);
            
            total_tests++;
            if (result.satisfiable) {
                total_pass++;
                std::cout << "  " << num_vars << " vars: SAT (iter=" 
                         << result.iterations << ", energy=" << result.energy << ")\n";
            } else {
                std::cout << "  " << num_vars << " vars: UNSAT (energy=" 
                         << result.energy << ")\n";
            }
        }
    }
    
    std::cout << "\nTotal: " << total_pass << "/" << total_tests << "\n";
    
    return 0;
}
