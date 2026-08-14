#include "src/np/golden_fibonacci_v2.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== GOLDEN FIBONACCI V2 SAT ===\n\n";
    std::random_device rd;
    std::mt19937 gen(42);

    for (int num_vars : {10, 50, 100, 200, 300, 500, 1000, 2000, 5000, 10000, 20000, 50000}) {
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

        auto start = std::chrono::high_resolution_clock::now();
        auto result = GoldenFibonacciV2::GoldenFibonacciV2Solver::solve(clauses, num_vars);
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << num_vars << " vars: " << (result.satisfiable ? "SAT" : "UNSAT")
                 << " (fib_assigns=" << result.fibonacci_assignments
                 << ", time=" << ms << "ms)\n";

        if (ms > 30000) {
            std::cout << "  TIMEOUT\n";
            break;
        }
    }

    return 0;
}
