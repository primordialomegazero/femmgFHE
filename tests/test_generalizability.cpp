#include "src/np/golden_refined.h"
#include <iostream>
#include <chrono>
#include <random>
#include <vector>

int main() {
    std::cout << "=== GENERALIZABILITY CHECK ===\n\n";

    int total_tests = 0;
    int total_correct = 0;

    // Test 1: Random 2-SAT (polynomial ang 2-SAT)
    std::cout << "--- 2-SAT ---\n";
    for (int num_vars : {10, 20, 30, 50}) {
        int num_clauses = num_vars * 5;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(100 + num_vars);

        for (int c = 0; c < num_clauses; c++) {
            int v1 = rng() % num_vars + 1;
            int v2 = rng() % num_vars + 1;
            if (rng() % 2) v1 = -v1;
            if (rng() % 2) v2 = -v2;
            clauses.push_back({v1, v2});
        }

        auto result = GoldenRefined::GoldenRefinedDetector::detect(clauses);
        std::cout << "  2-SAT vars=" << num_vars << ": unsat=" << result.unsatisfiable << "\n";
        total_tests++;
    }

    // Test 2: Random 4-SAT
    std::cout << "\n--- 4-SAT ---\n";
    for (int num_vars : {10, 20, 30}) {
        int num_clauses = num_vars * 3;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(200 + num_vars);

        for (int c = 0; c < num_clauses; c++) {
            std::vector<int> clause;
            for (int k = 0; k < 4; k++) {
                int v = rng() % num_vars + 1;
                if (rng() % 2) v = -v;
                clause.push_back(v);
            }
            clauses.push_back(clause);
        }

        auto result = GoldenRefined::GoldenRefinedDetector::detect(clauses);
        std::cout << "  4-SAT vars=" << num_vars << ": unsat=" << result.unsatisfiable << "\n";
        total_tests++;
    }

    // Test 3: 5-SAT
    std::cout << "\n--- 5-SAT ---\n";
    for (int num_vars : {10, 20}) {
        int num_clauses = num_vars * 2;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(300 + num_vars);

        for (int c = 0; c < num_clauses; c++) {
            std::vector<int> clause;
            for (int k = 0; k < 5; k++) {
                int v = rng() % num_vars + 1;
                if (rng() % 2) v = -v;
                clause.push_back(v);
            }
            clauses.push_back(clause);
        }

        auto result = GoldenRefined::GoldenRefinedDetector::detect(clauses);
        std::cout << "  5-SAT vars=" << num_vars << ": unsat=" << result.unsatisfiable << "\n";
        total_tests++;
    }

    // Test 4: Horn SAT (kilalang polynomial-time)
    std::cout << "\n--- HORN SAT ---\n";
    for (int num_vars : {5, 10, 15}) {
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(400 + num_vars);

        for (int c = 0; c < num_vars * 3; c++) {
            std::vector<int> clause;
            // Horn: hindi hihigit sa isang positive literal
            int positive_idx = rng() % 3;
            for (int k = 0; k < 3; k++) {
                int v = rng() % num_vars + 1;
                if (k == positive_idx) {
                    clause.push_back(v);
                } else {
                    clause.push_back(-v);
                }
            }
            clauses.push_back(clause);
        }

        auto result = GoldenRefined::GoldenRefinedDetector::detect(clauses);
        std::cout << "  Horn vars=" << num_vars << ": unsat=" << result.unsatisfiable << "\n";
        total_tests++;
    }

    // Test 5: XOR SAT (linear algebra)
    std::cout << "\n--- XOR SAT ---\n";
    for (int num_vars : {5, 10}) {
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(500 + num_vars);

        for (int c = 0; c < num_vars * 2; c++) {
            std::vector<int> clause;
            for (int k = 0; k < 3; k++) {
                int v = rng() % num_vars + 1;
                if (rng() % 2) v = -v;
                clause.push_back(v);
            }
            clauses.push_back(clause);
        }

        auto result = GoldenRefined::GoldenRefinedDetector::detect(clauses);
        std::cout << "  XOR vars=" << num_vars << ": unsat=" << result.unsatisfiable << "\n";
        total_tests++;
    }

    std::cout << "\n============================\n";
    std::cout << "Total tests: " << total_tests << "\n";
    std::cout << "Detector runs: " << total_tests << " (100%)\n";

    return 0;
}
