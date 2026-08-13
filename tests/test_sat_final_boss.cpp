#include "src/np/golden_sat.h"
#include <iostream>
#include <chrono>
#include <random>

int main() {
    std::cout << "=== FINAL BOSS SAT ===\n\n";

    // Boss 1: 500 variables, satisfiable
    {
        int num_vars = 500;
        int num_clauses = 2000;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(42);
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

        std::cout << "BOSS 1 (500 vars, satisfiable):\n";
        std::cout << "  Sat: " << result.satisfiable
                  << " Steps: " << result.steps
                  << " Time: " << secs << "s\n\n";
    }

    // Boss 2: 1000 variables, satisfiable
    {
        int num_vars = 1000;
        int num_clauses = 4000;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(43);
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

        std::cout << "BOSS 2 (1000 vars, satisfiable):\n";
        std::cout << "  Sat: " << result.satisfiable
                  << " Steps: " << result.steps
                  << " Time: " << secs << "s\n\n";
    }

    // Boss 3: 2000 variables, satisfiable
    {
        int num_vars = 2000;
        int num_clauses = 8000;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(44);
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

        std::cout << "BOSS 3 (2000 vars, satisfiable):\n";
        std::cout << "  Sat: " << result.satisfiable
                  << " Steps: " << result.steps
                  << " Time: " << secs << "s\n";
    }

    return 0;
}
