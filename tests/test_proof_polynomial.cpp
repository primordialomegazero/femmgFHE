#include "src/np/golden_refined.h"
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>

int main() {
    std::cout << "=== PROOF SIZE POLYNOMIAL VERIFICATION ===\n\n";

    // I-test sa iba't ibang laki at ratios
    for (int num_vars : {100, 200, 500, 1000, 2000}) {
        for (double ratio : {3.0, 4.26, 5.0}) {
            int num_clauses = (int)(num_vars * ratio);
            std::vector<std::vector<int>> clauses;
            std::mt19937_64 rng(num_vars * 100 + (int)(ratio * 100));

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
            auto result = GoldenRefined::GoldenRefinedDetector::detect(clauses);
            auto t1 = std::chrono::high_resolution_clock::now();
            double secs = std::chrono::duration<double>(t1 - t0).count();

            std::cout << "Vars=" << num_vars << " Ratio=" << ratio
                      << " unsat=" << result.unsatisfiable
                      << " steps=" << result.steps
                      << " time=" << secs << "s\n";
        }
        std::cout << "\n";
    }

    // Scaling analysis
    std::cout << "--- SCALING (ratio=4.26) ---\n";
    std::vector<std::pair<int, double>> timings;
    for (int num_vars : {100, 200, 400, 800, 1600}) {
        int num_clauses = (int)(num_vars * 4.26);
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
        auto result = GoldenRefined::GoldenRefinedDetector::detect(clauses);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();
        timings.push_back({num_vars, secs});
        std::cout << "  " << num_vars << " vars: " << secs << "s\n";
    }

    std::cout << "\n  Scaling ratios:\n";
    for (size_t i = 1; i < timings.size(); i++) {
        double ratio = timings[i].second / timings[i-1].second;
        std::cout << "  " << timings[i].first << "/" << timings[i-1].first
                  << ": " << ratio << "x\n";
    }

    return 0;
}
