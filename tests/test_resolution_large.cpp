#include "src/np/golden_resolution.h"
#include <iostream>
#include <chrono>
#include <vector>

int main() {
    std::cout << "=== GOLDEN RESOLUTION — LARGE PIGEONHOLE ===\n\n";

    for (int holes : {6, 7, 8, 9, 10}) {
        int pigeons = holes + 1;
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

        int total_vars = holes * pigeons;
        int total_clauses = clauses.size();

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenResolution::GoldenResolutionProver::prove_unsat(clauses);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "Pigeonhole (" << holes << "/" << pigeons << "): ";
        std::cout << "vars=" << total_vars << " clauses=" << total_clauses << " ";
        std::cout << "unsat=" << result.unsatisfiable << " ";
        std::cout << "resolutions=" << result.resolution_steps << " ";
        std::cout << "time=" << secs << "s\n";

        if (secs > 60) {
            std::cout << "  ^^ Masyadong malaki na — stopping\n";
            break;
        }
    }

    return 0;
}
