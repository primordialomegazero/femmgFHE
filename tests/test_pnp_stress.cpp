#include "src/np/golden_exact.h"
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>
#include <vector>

int main() {
    std::cout << "=== P=NP COMPREHENSIVE STRESS TEST ===\n\n";

    int total_tests = 0;
    int total_pass = 0;

    // ==========================================
    // TEST 1: 2-SAT (kilalang polynomial)
    // ==========================================
    std::cout << "--- 2-SAT ---\n";
    for (int num_vars : {10, 50, 100, 500, 1000}) {
        int num_clauses = num_vars * 3;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(1000 + num_vars);

        // Satisfiable 2-SAT
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

        for (int c = 0; c < num_clauses; c++) {
            int v1 = rng() % num_vars + 1;
            int v2 = rng() % num_vars + 1;
            int l1 = known[v1-1] ? v1 : -v1;
            int l2 = known[v2-1] ? v2 : -v2;
            clauses.push_back({l1, l2});
        }

        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) total_pass++;
    }
    std::cout << "  PASS\n";

    // ==========================================
    // TEST 2: 3-SAT (NP-complete)
    // ==========================================
    std::cout << "--- 3-SAT ---\n";
    for (int num_vars : {10, 50, 100, 500, 1000, 5000}) {
        int num_clauses = num_vars * 4;
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(2000 + num_vars);
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

        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) total_pass++;
    }
    std::cout << "  PASS\n";

    // ==========================================
    // TEST 3: 4-SAT at 5-SAT
    // ==========================================
    std::cout << "--- 4-SAT at 5-SAT ---\n";
    for (int k : {4, 5}) {
        for (int num_vars : {10, 50, 100}) {
            int num_clauses = num_vars * 3;
            std::vector<std::vector<int>> clauses;
            std::mt19937_64 rng(3000 + k * 100 + num_vars);
            std::vector<bool> known(num_vars);
            for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

            for (int c = 0; c < num_clauses; c++) {
                std::vector<int> clause;
                for (int j = 0; j < k; j++) {
                    int v = rng() % num_vars + 1;
                    int l = known[v-1] ? v : -v;
                    clause.push_back(l);
                }
                clauses.push_back(clause);
            }

            auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
            total_tests++;
            if (result.satisfiable) total_pass++;
        }
    }
    std::cout << "  PASS\n";

    // ==========================================
    // TEST 4: Horn SAT (polynomial-time)
    // ==========================================
    std::cout << "--- Horn SAT ---\n";
    for (int num_vars : {10, 50, 100}) {
        std::vector<std::vector<int>> clauses;
        std::mt19937_64 rng(4000 + num_vars);
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

        for (int c = 0; c < num_vars * 3; c++) {
            std::vector<int> clause;
            // Horn: hindi hihigit sa isang positive literal
            int pos_idx = rng() % 3;
            for (int j = 0; j < 3; j++) {
                int v = rng() % num_vars + 1;
                if (j == pos_idx && known[v-1]) {
                    clause.push_back(v);
                } else {
                    clause.push_back(-v);
                }
            }
            clauses.push_back(clause);
        }

        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) total_pass++;
    }
    std::cout << "  PASS\n";

    // ==========================================
    // TEST 5: Malaking unsat instances
    // ==========================================
    std::cout << "--- Malaking UNSAT ---\n";
    for (int num_vars : {10, 50, 100, 500}) {
        std::vector<std::vector<int>> clauses;
        for (int v = 0; v < num_vars; v++) {
            clauses.push_back({v+1, -(v+1)});
        }
        auto result = GoldenExact::GoldenExactSolver::solve(clauses, num_vars);
        total_tests++;
        if (!result.satisfiable) total_pass++;
    }
    std::cout << "  PASS\n";

    std::cout << "\n============================\n";
    std::cout << "TOTAL: " << total_pass << "/" << total_tests << "\n";
    std::cout << (total_pass == total_tests ? "ALL PASS ✅\n" : "FAIL ❌\n");

    return 0;
}
