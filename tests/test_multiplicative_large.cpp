#include "src/np/golden_multiplicative.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== MULTIPLICATIVE SOLVER — LARGE SCALE ===\n\n";

    int total_tests = 0;
    int total_pass = 0;

    // ==========================================
    // GRAPH COLORING (100 → 1000 vertices)
    // ==========================================
    std::cout << "--- Graph Coloring ---\n";
    for (int num_vertices : {10, 20, 50, 100, 200}) {
        std::vector<std::vector<int>> graph(num_vertices);
        std::mt19937_64 rng(42 + num_vertices);
        for (int i = 0; i < num_vertices; i++) {
            for (int j = i + 1; j < num_vertices; j++) {
                if (rng() % 4 == 0) {
                    graph[i].push_back(j);
                    graph[j].push_back(i);
                }
            }
        }

        int num_vars = num_vertices * 3;
        std::vector<std::vector<int>> clauses;

        for (int v = 0; v < num_vertices; v++) {
            clauses.push_back({v*3 + 1, v*3 + 2, v*3 + 3});
            clauses.push_back({-(v*3 + 1), -(v*3 + 2)});
            clauses.push_back({-(v*3 + 1), -(v*3 + 3)});
            clauses.push_back({-(v*3 + 2), -(v*3 + 3)});
        }

        for (int v1 = 0; v1 < num_vertices; v1++) {
            for (int v2 : graph[v1]) {
                if (v1 < v2) {
                    for (int c = 1; c <= 3; c++) {
                        clauses.push_back({-(v1*3 + c), -(v2*3 + c)});
                    }
                }
            }
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenMultiplicative::GoldenMultiplicativeSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        total_tests++;
        if (result.satisfiable) total_pass++;
        std::cout << "  " << num_vertices << " vertices: sat=" << result.satisfiable
                  << " time=" << secs << "s\n";
    }

    // ==========================================
    // SCHEDULING (10 → 100 tasks)
    // ==========================================
    std::cout << "\n--- Scheduling ---\n";
    for (int num_tasks : {5, 10, 20, 50, 100}) {
        int num_machines = 3;
        int num_vars = num_tasks * num_machines;
        std::vector<std::vector<int>> clauses;

        for (int t = 0; t < num_tasks; t++) {
            clauses.push_back({t * num_machines + 1, t * num_machines + 2, t * num_machines + 3});
            clauses.push_back({-(t * num_machines + 1), -(t * num_machines + 2)});
            clauses.push_back({-(t * num_machines + 1), -(t * num_machines + 3)});
            clauses.push_back({-(t * num_machines + 2), -(t * num_machines + 3)});
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenMultiplicative::GoldenMultiplicativeSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        total_tests++;
        if (result.satisfiable) total_pass++;
        std::cout << "  " << num_tasks << " tasks: sat=" << result.satisfiable
                  << " time=" << secs << "s\n";
    }

    std::cout << "\n============================\n";
    std::cout << "TOTAL: " << total_pass << "/" << total_tests << "\n";
    std::cout << (total_pass == total_tests ? "ALL PASS ✅\n" : "FAIL ❌\n");

    return 0;
}
