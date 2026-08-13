#include "src/np/golden_resolution.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "=== GOLDEN RESOLUTION PROOF ===\n\n";

    // Simpleng unsat: (x) AND (NOT x)
    {
        std::vector<std::vector<int>> clauses = {{1}, {-1}};
        auto result = GoldenResolution::GoldenResolutionProver::prove_unsat(clauses);
        std::cout << "Simple unsat: " << result.unsatisfiable
                  << " (expect 1) steps=" << result.steps << "\n";
    }

    // Pigeonhole: 3 holes, 4 pigeons
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

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenResolution::GoldenResolutionProver::prove_unsat(clauses);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "Pigeonhole (3/4): unsat=" << result.unsatisfiable
                  << " steps=" << result.steps
                  << " resolutions=" << result.resolution_steps
                  << " time=" << secs << "s\n";
    }

    // Pigeonhole: 5 holes, 6 pigeons
    {
        int holes = 5;
        int pigeons = 6;
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

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenResolution::GoldenResolutionProver::prove_unsat(clauses);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "Pigeonhole (5/6): unsat=" << result.unsatisfiable
                  << " steps=" << result.steps
                  << " resolutions=" << result.resolution_steps
                  << " time=" << secs << "s\n";
    }

    return 0;
}
