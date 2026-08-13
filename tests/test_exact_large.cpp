#include "src/np/golden_exact.h"
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>

int main() {
    std::cout << "=== GOLDEN EXACT — LARGE SCALE ===\n\n";

    // Satisfiable instances: 100 → 10,000 variables
    std::cout << "--- SATISFIABLE (100 → 10K vars) ---\n";
    std::vector<std::pair<int, double>> timings;

    for (int num_vars : {100, 500, 1000, 2000, 5000, 10000}) {
        int num_clauses = num_vars * 4;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(num_vars);
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

        for (int c = 0; c < num_clauses; c++) {
            int v1 = rng() % num_vars + 1;
            int v2 = rng() % num_vars + 1;
            int v3 = rng() % num_vars + 1;
            int l1 = known[v1-1] ? v1 : -v1;
            int l2 = known[v2-1] ? v2 : -v2;
            int l3 = known[v3-1] ? v3 : -v3;
            clauses.push_back({l1, l2, l3});
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();
        timings.push_back({num_vars, secs});

        std::cout << "  " << num_vars << " vars: sat=" << result.satisfiable
                  << " time=" << secs << "s\n";
    }

    // Scaling analysis
    std::cout << "\n--- SCALING ANALYSIS ---\n";
    for (size_t i = 1; i < timings.size(); i++) {
        double ratio = timings[i].second / timings[i-1].second;
        double size_ratio = (double)timings[i].first / timings[i-1].first;
        std::cout << "  " << timings[i].first << "/" << timings[i-1].first
                  << ": time_ratio=" << ratio << "x size_ratio=" << size_ratio << "x\n";
    }

    // Test sa unsat instances
    std::cout << "\n--- UNSATISFIABLE ---\n";
    for (int num_vars : {10, 20, 50, 100}) {
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(999 + num_vars);

        // Gumawa ng unsat: (x1 OR x2) AND (NOT x1 OR x2) AND (x1 OR NOT x2) AND (NOT x1 OR NOT x2)
        for (int c = 0; c < num_vars; c++) {
            clauses.push_back({c+1, c+2});
            clauses.push_back({-(c+1), c+2});
            clauses.push_back({c+1, -(c+2)});
            clauses.push_back({-(c+1), -(c+2)});
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars + 1);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "  " << num_vars << " vars: sat=" << result.satisfiable
                  << " time=" << secs << "s\n";
    }

    return 0;
}
