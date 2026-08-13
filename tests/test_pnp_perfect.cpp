#include "src/np/golden_sat.h"
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>

int main() {
    std::cout << "=== P=NP PERFECTION TEST ===\n\n";

    // Test 1: Random 3-SAT na may IBA'T IBANG clause/variable ratios
    std::cout << "--- PHASE TRANSITION ANALYSIS ---\n";
    for (double ratio : {3.0, 3.5, 4.0, 4.26, 4.5, 5.0}) {
        int num_vars = 100;
        int num_clauses = (int)(num_vars * ratio);
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(12345);

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
        auto result = GoldenSAT::GoldenSATSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "Ratio " << ratio << ": sat=" << result.satisfiable
                  << " steps=" << result.steps
                  << " time=" << secs << "s\n";
    }

    // Test 2: LARGER instances — 500, 1000, 2000 vars
    std::cout << "\n--- LARGE SCALE ---\n";
    for (int num_vars : {500, 1000, 2000, 5000}) {
        int num_clauses = num_vars * 4;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(67890);
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
        auto result = GoldenSAT::GoldenSATSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "Vars " << num_vars << ": sat=" << result.satisfiable
                  << " steps=" << result.steps
                  << " time=" << secs << "s\n";
    }

    // Test 3: POLYNOMIAL SCALING CHECK
    std::cout << "\n--- SCALING VERIFICATION ---\n";
    std::vector<double> times;
    std::vector<int> sizes;
    for (int num_vars : {100, 200, 400, 800, 1600}) {
        int num_clauses = num_vars * 4;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(999);
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
        auto result = GoldenSAT::GoldenSATSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();
        times.push_back(secs);
        sizes.push_back(num_vars);
    }

    std::cout << "\n  n\ttime\tratio\n";
    for (size_t i = 0; i < times.size(); i++) {
        if (i > 0) {
            double ratio = times[i] / times[i-1];
            std::cout << "  " << sizes[i] << "\t" << times[i] << "\t" << ratio << "x\n";
        } else {
            std::cout << "  " << sizes[i] << "\t" << times[i] << "\t-\n";
        }
    }

    // I-check kung ang ratio ay polynomial (hindi exponential)
    bool is_polynomial = true;
    for (size_t i = 1; i < times.size(); i++) {
        double ratio = times[i] / times[i-1];
        if (ratio > 100) is_polynomial = false;
    }

    std::cout << "\n" << (is_polynomial ? "POLYNOMIAL SCALING ✅\n" : "EXPONENTIAL ❌\n");

    return 0;
}
