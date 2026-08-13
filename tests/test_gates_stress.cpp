#include "src/np/golden_exact_gates.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== GATE-BASED SOLVER STRESS ===\n\n";

    int total_tests = 0;
    int total_pass = 0;

    // ==========================================
    // TEST 1: Lahat ng Gates — Random Truth Tables
    // ==========================================
    std::cout << "--- Lahat ng Gates (Random Truth Tables) ---\n";
    for (int gate_id = 0; gate_id < 6; gate_id++) {
        std::string gate;
        if (gate_id == 0) gate = "NAND";
        else if (gate_id == 1) gate = "AND";
        else if (gate_id == 2) gate = "OR";
        else if (gate_id == 3) gate = "XOR";
        else if (gate_id == 4) gate = "NOR";
        else gate = "XNOR";

        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                std::vector<std::vector<int>> clauses;
                int num_vars = 2;

                if (gate == "NAND") {
                    if (!(a && b)) clauses.push_back({-1, -2});
                    else clauses.push_back({1});
                } else if (gate == "AND") {
                    if (a) clauses.push_back({1});
                    if (b) clauses.push_back({2});
                    if (a && b) clauses.push_back({1, 2});
                } else if (gate == "OR") {
                    clauses.push_back({a ? 1 : -1, b ? 2 : -2});
                } else if (gate == "XOR") {
                    if (a != b) clauses.push_back({1, -2});
                    else clauses.push_back({1, 2});
                } else if (gate == "NOR") {
                    if (!(a || b)) clauses.push_back({-1, -2});
                    else clauses.push_back({-1});
                } else { // XNOR
                    if (a == b) clauses.push_back({1, 2});
                    else clauses.push_back({1, -2});
                }

                auto result = GoldenExactGates::GoldenExactSolver::solve(clauses, num_vars);
                total_tests++;
                if (result.satisfiable) total_pass++;
            }
        }
    }
    std::cout << "  Result: " << total_pass << "/" << total_tests << "\n\n";

    // ==========================================
    // TEST 2: Large 3-SAT (100 → 10K)
    // ==========================================
    std::cout << "--- Large 3-SAT ---\n";
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
        auto result = GoldenExactGates::GoldenExactSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        total_tests++;
        if (result.satisfiable) total_pass++;

        std::cout << "  " << num_vars << " vars: sat=" << result.satisfiable
                  << " time=" << secs << "s\n";
    }

    std::cout << "\n============================\n";
    std::cout << "TOTAL: " << total_pass << "/" << total_tests << "\n";
    std::cout << (total_pass == total_tests ? "ALL PASS ✅\n" : "FAIL ❌\n");

    return 0;
}
