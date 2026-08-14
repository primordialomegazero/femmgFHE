#include "src/np/golden_watched_v2.h"
#include <iostream>
#include <random>
#include <chrono>

// Global counters para sa profiling
static long long propagate_calls = 0;
static long long select_calls = 0;
static long long branch_calls = 0;
static long long clause_checks = 0;

int main() {
    std::cout << "=== FUNCTION PROFILING ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);
    
    // Test sa 500 vars para mabilis
    int num_vars = 500;
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
    
    std::cout << "Solving " << num_vars << " vars...\n\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    auto result = GoldenWatchedV2::GoldenWatchedSolverV2::solve(clauses, num_vars);
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Result: " << (result.satisfiable ? "SAT" : "UNSAT") << "\n";
    std::cout << "Steps: " << result.steps << "\n";
    std::cout << "Conflicts: " << result.conflicts << "\n";
    std::cout << "Time: " << ms << "ms\n";
    
    // Estimate
    std::cout << "\n=== ESTIMATES ===\n";
    std::cout << "Clauses scanned per step: ~" << num_clauses << "\n";
    std::cout << "Total clause checks: ~" << (long long)num_clauses * result.steps << "\n";
    std::cout << "Time per clause check: " << (double)ms / (num_clauses * result.steps) * 1000 << " microseconds\n";
    
    return 0;
}
