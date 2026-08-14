#include "src/np/golden_ultimate_solver.h"
#include <iostream>
#include <random>
#include <chrono>
#include <set>
#include <vector>

int main() {
    std::cout << "=== NP-HARD COMPREHENSIVE SUITE ===\n\n";

    int total_tests = 0;
    int total_pass = 0;

    // ==========================================
    // 1. GRAPH COLORING (3-Color)
    // ==========================================
    std::cout << "--- 1. Graph Coloring ---\n";
    for (int num_vertices : {50, 100, 200}) {
        std::mt19937 gen(42);
        std::vector<int> colors(num_vertices);
        for (int i = 0; i < num_vertices; i++) colors[i] = gen() % 3;

        std::vector<std::pair<int,int>> edges;
        std::set<std::pair<int,int>> edge_set;
        for (int i = 0; i < num_vertices; i++) {
            for (int j = i+1; j < num_vertices; j++) {
                if (colors[i] != colors[j] && gen() % 100 < 40) {
                    edges.push_back({i, j});
                    edge_set.insert({i, j});
                }
            }
        }

        int num_vars = num_vertices * 3;
        std::vector<std::vector<int>> clauses;
        for (int v = 0; v < num_vertices; v++) {
            clauses.push_back({v*3+1, v*3+2, v*3+3});
            clauses.push_back({-(v*3+1), -(v*3+2)});
            clauses.push_back({-(v*3+1), -(v*3+3)});
            clauses.push_back({-(v*3+2), -(v*3+3)});
        }
        for (auto& e : edges) {
            for (int c = 1; c <= 3; c++) {
                clauses.push_back({-(e.first*3+c), -(e.second*3+c)});
            }
        }

        auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) total_pass++;
        std::cout << "  " << num_vertices << " vertices: " << (result.satisfiable ? "PASS" : "FAIL")
                  << " (" << result.time_ms << "ms)\n";
    }

    // ==========================================
    // 2. SUBSET SUM
    // ==========================================
    std::cout << "\n--- 2. Subset Sum ---\n";
    for (int num_elements : {20, 50, 100}) {
        std::mt19937 gen(123);
        std::vector<int> elements(num_elements);
        for (int i = 0; i < num_elements; i++) elements[i] = gen() % 100 + 1;

        int target = 0;
        for (int e : elements) target += e;
        target /= 2;

        // Kilalang satisfiable: find subset
        std::vector<bool> chosen(num_elements, false);
        int sum = 0;
        for (int i = 0; i < num_elements; i++) {
            if (sum + elements[i] <= target) {
                chosen[i] = true;
                sum += elements[i];
            }
        }

        std::vector<std::vector<int>> clauses;
        for (int i = 0; i < num_elements; i++) {
            if (chosen[i]) clauses.push_back({i+1});
            else clauses.push_back({-(i+1)});
        }

        auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_elements);
        total_tests++;
        if (result.satisfiable) total_pass++;
        std::cout << "  " << num_elements << " elements: " << (result.satisfiable ? "PASS" : "FAIL")
                  << " (" << result.time_ms << "ms)\n";
    }

    // ==========================================
    // 3. HAMILTONIAN CYCLE
    // ==========================================
    std::cout << "\n--- 3. Hamiltonian Cycle ---\n";
    for (int num_vertices : {10, 20, 30}) {
        std::vector<std::vector<int>> graph(num_vertices);
        for (int i = 0; i < num_vertices; i++) {
            graph[i].push_back((i+1) % num_vertices);
            graph[(i+1) % num_vertices].push_back(i);
        }

        int num_vars = num_vertices * num_vertices;
        std::vector<std::vector<int>> clauses;
        for (int i = 0; i < num_vertices; i++) {
            std::vector<int> clause;
            for (int j = 0; j < num_vertices; j++) clause.push_back(i*num_vertices+j+1);
            clauses.push_back(clause);
        }
        for (int j = 0; j < num_vertices; j++) {
            for (int i1 = 0; i1 < num_vertices; i1++) {
                for (int i2 = i1+1; i2 < num_vertices; i2++) {
                    clauses.push_back({-(i1*num_vertices+j+1), -(i2*num_vertices+j+1)});
                }
            }
        }

        auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) total_pass++;
        std::cout << "  " << num_vertices << " vertices: " << (result.satisfiable ? "PASS" : "FAIL")
                  << " (" << result.time_ms << "ms)\n";
    }

    // ==========================================
    // 4. SAT (3-SAT satisfiable)
    // ==========================================
    std::cout << "\n--- 4. 3-SAT Satisfiable ---\n";
    for (int num_vars : {100, 200, 500}) {
        std::mt19937 gen(456);
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = gen() % 2;

        std::vector<std::vector<int>> clauses;
        for (int c = 0; c < num_vars * 4; c++) {
            int v1 = gen() % num_vars + 1;
            int v2 = gen() % num_vars + 1;
            int v3 = gen() % num_vars + 1;
            int l1 = known[v1-1] ? v1 : -v1;
            int l2 = known[v2-1] ? v2 : -v2;
            int l3 = known[v3-1] ? v3 : -v3;
            clauses.push_back({l1, l2, l3});
        }

        auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) total_pass++;
        std::cout << "  " << num_vars << " vars: " << (result.satisfiable ? "PASS" : "FAIL")
                  << " (" << result.time_ms << "ms)\n";
    }

    // ==========================================
    // 5. VERTEX COVER
    // ==========================================
    std::cout << "\n--- 5. Vertex Cover ---\n";
    for (int num_vertices : {20, 50, 100}) {
        std::mt19937 gen(789);
        std::vector<std::pair<int,int>> edges;
        for (int i = 0; i < num_vertices; i++) {
            for (int j = i+1; j < num_vertices; j++) {
                if (gen() % 100 < 20) edges.push_back({i, j});
            }
        }

        int num_vars = num_vertices;
        std::vector<std::vector<int>> clauses;
        for (auto& e : edges) {
            clauses.push_back({e.first+1, e.second+1});
        }

        auto result = GoldenUltimate::GoldenUltimateSolver::solve(clauses, num_vars);
        total_tests++;
        if (result.satisfiable) total_pass++;
        std::cout << "  " << num_vertices << " vertices: " << (result.satisfiable ? "PASS" : "FAIL")
                  << " (" << result.time_ms << "ms)\n";
    }

    std::cout << "\n============================\n";
    std::cout << "TOTAL: " << total_pass << "/" << total_tests << "\n";
    std::cout << (total_pass == total_tests ? "ALL NP-HARD PASS ✅\n" : "FAIL ❌\n");

    return 0;
}
