#include "src/np/golden_watched_v2.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== DEBUG: SAAN BA ANG BOTTLENECK? ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    // Test sa 1000 vars para makita ang bottleneck
    int num_vars = 1000;
    int num_clauses = num_vars * 3;
    
    std::cout << "Generating " << num_vars << " vars, " << num_clauses << " clauses...\n";
    
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
    
    std::cout << "Done generating.\n\n";
    std::cout << "Solving...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    auto result = GoldenWatchedV2::GoldenWatchedSolverV2::solve(clauses, num_vars);
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Result: " << (result.satisfiable ? "SAT" : "UNSAT") << "\n";
    std::cout << "Steps: " << result.steps << "\n";
    std::cout << "Conflicts: " << result.conflicts << "\n";
    std::cout << "Time: " << ms << "ms\n";
    std::cout << "Time per step: " << (double)ms / result.steps << "ms\n";
    std::cout << "Time per conflict: " << (double)ms / (result.conflicts + 1) << "ms\n";
    
    // Analyze ang bottleneck
    std::cout << "\n=== ANALYSIS ===\n";
    std::cout << "Kung time per step ay mataas (>10ms), ang problema ay sa propagation\n";
    std::cout << "Kung conflicts ay mataas (>100), ang problema ay sa variable selection\n";
    std::cout << "Kung steps ay mataas (>10000), ang problema ay sa branching\n";
    
    return 0;
}
