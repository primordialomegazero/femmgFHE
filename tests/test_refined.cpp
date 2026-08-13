#include "src/np/golden_refined.h"
#include <iostream>
#include <chrono>
#include <random>

int main() {
    std::cout << "=== REFINED GOLDEN DETECTOR ===\n\n";

    // Test 1: Unit contradiction — dapat verified
    {
        std::vector<std::vector<int>> clauses = {{1}, {-1}};
        auto result = GoldenRefined::GoldenRefinedDetector::detect(clauses);
        std::cout << "Unit: unsat=" << result.unsatisfiable
                  << " verified=" << result.verified << "\n";
    }

    // Test 2: Tunay na pigeonhole — dapat verified
    {
        int holes = 3;
        int pigeons = 4;
        std::vector<std::vector<int>> clauses;
        for (int p = 0; p < pigeons; p++) {
            std::vector<int> clause;
            for (int h = 0; h < holes; h++) {
                clause.push_back(p * holes + h + 1);
            }
            clauses.push_back(clause);
        }
        for (int h = 0; h < holes; h++) {
            for (int p1 = 0; p1 < pigeons; p1++) {
                for (int p2 = p1 + 1; p2 < pigeons; p2++) {
                    clauses.push_back({-(p1 * holes + h + 1), -(p2 * holes + h + 1)});
                }
            }
        }

        auto result = GoldenRefined::GoldenRefinedDetector::detect(clauses);
        std::cout << "Pigeonhole: unsat=" << result.unsatisfiable
                  << " verified=" << result.verified << "\n";
    }

    // Test 3: Random 3-SAT na may iba't ibang ratios
    std::cout << "\n--- RANDOM 3-SAT ---\n";
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

        auto result = GoldenRefined::GoldenRefinedDetector::detect(clauses);
        std::cout << "Ratio " << ratio << ": unsat=" << result.unsatisfiable
                  << " verified=" << result.verified << "\n";
    }

    return 0;
}
