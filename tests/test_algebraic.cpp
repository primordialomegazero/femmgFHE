#include "src/np/golden_algebraic.h"
#include <iostream>
#include <chrono>
#include <random>

int main() {
    std::cout << "=== GOLDEN ALGEBRAIC CONTRADICTION ===\n\n";

    // Test 1: Simpleng unsat
    {
        std::vector<std::vector<int>> clauses = {{1}, {-1}};
        auto result = GoldenAlgebraic::GoldenAlgebraicDetector::detect(clauses);
        std::cout << "Unit contradiction: " << result.unsatisfiable
                  << " (" << result.reason << ")\n";
    }

    // Test 2: Random 3-SAT na may iba't ibang ratios
    for (double ratio : {3.0, 4.0, 4.26, 5.0}) {
        int num_vars = 100;
        int num_clauses = (int)(num_vars * ratio);
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(42);

        for (int c = 0; c < num_clauses; c++) {
            int v1 = rng() % num_vars + 1;
            int v2 = rng() % num_vars + 1;
            int v3 = rng() % num_vars + 1;
            if (rng() % 2) v1 = -v1;
            if (rng() % 2) v2 = -v2;
            if (rng() % 2) v3 = -v3;
            clauses.push_back({v1, v2, v3});
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenAlgebraic::GoldenAlgebraicDetector::detect(clauses);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "Ratio " << ratio << ": unsat=" << result.unsatisfiable
                  << " steps=" << result.steps
                  << " time=" << secs << "s";
        if (result.unsatisfiable) {
            std::cout << " (" << result.reason << ")";
        }
        std::cout << "\n";
    }

    return 0;
}
