#include "src/np/golden_propagating_optimized.h"
#include <iostream>
#include <random>

int main() {
    std::cout << "=== TRACE COLLAPSE (50 vars) ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    int num_vars = 50;
    int num_clauses = num_vars * 3;
    
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
    
    std::cout << "Clauses (unang 10):\n";
    for (int i = 0; i < 10 && i < clauses.size(); i++) {
        std::cout << "  ";
        for (int lit : clauses[i]) std::cout << lit << " ";
        std::cout << "\n";
    }
    
    auto result = GoldenPropagatingOptimized::GoldenCollapseSolver::solve(clauses, num_vars);
    
    std::cout << "\nResult: " << (result.satisfiable ? "SAT" : "UNSAT") << "\n";
    std::cout << "Steps: " << result.steps << "\n";
    std::cout << "Assignment (unang 20): ";
    for (int i = 0; i < 20 && i < result.assignment.size(); i++) {
        std::cout << result.assignment[i] << " ";
    }
    std::cout << "\n";
    
    return 0;
}
