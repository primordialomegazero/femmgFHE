#include "src/np/golden_sat.h"
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>

int main() {
    std::cout << "=== SAT COMPLETE VERIFICATION ===\n\n";

    // Test 1: Unsatisfiable instances
    std::cout << "--- UNSATISFIABLE ---\n";
    {
        // (x1) AND (NOT x1) — unsat
        std::vector<std::vector<int>> unsat = {{1}, {-1}};
        auto result = GoldenSAT::GoldenSATSolver::solve(unsat, 1);
        std::cout << "Simple unsat: " << result.satisfiable << " (expect 0)\n";

        // 3-SAT unsat na may phase transition
        for (int num_vars : {10, 20, 30}) {
            int num_clauses = (int)(num_vars * 5.0); // 5x ratio = mostly unsat
            std::vector<std::vector<int>> clauses;
            std::mt19937_64 rng(999);

            for (int c = 0; c < num_clauses; c++) {
                int v1 = rng() % num_vars + 1;
                int v2 = rng() % num_vars + 1;
                int v3 = rng() % num_vars + 1;
                if (rng() % 2) v1 = -v1;
                if (rng() % 2) v2 = -v2;
                if (rng() % 2) v3 = -v3;
                clauses.push_back({v1, v2, v3});
            }

            auto result2 = GoldenSAT::GoldenSATSolver::solve(clauses, num_vars);
            std::cout << "Unsat " << num_vars << " vars: " << result2.satisfiable << "\n";
        }
    }

    std::cout << "\n--- SATISFIABLE (RANDOM) ---\n";
    {
        for (int num_vars : {50, 100, 200}) {
            int num_clauses = num_vars * 4;
            std::vector<std::vector<int>> clauses;
            std::mt19937_64 rng(123);

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

            std::cout << "Sat " << num_vars << " vars: " << result.satisfiable
                      << " (time " << secs << "s)\n";
        }
    }

    std::cout << "\n--- SCALING ANALYSIS ---\n";
    {
        // I-verify kung ang time ay polynomial hindi exponential
        std::vector<std::pair<int, double>> timings;
        for (int num_vars : {100, 200, 400, 800}) {
            int num_clauses = num_vars * 4;
            std::vector<std::vector<int>> clauses;
            std::mt19937_64 rng(456);

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

            timings.push_back({num_vars, secs});
            std::cout << "  " << num_vars << " vars: " << secs << "s\n";
        }

        // I-check kung ang ratio ay polynomial
        std::cout << "\n  Scaling ratio (t2/t1):\n";
        for (size_t i = 1; i < timings.size(); i++) {
            double ratio = timings[i].second / timings[i-1].second;
            std::cout << "  " << timings[i].first << "/" << timings[i-1].first
                      << ": " << ratio << "x\n";
        }
    }

    return 0;
}
