#include "src/np/golden_sat.h"
#include <iostream>
#include <random>
#include <chrono>

int main() {
    std::cout << "=== SAT VERIFICATION ===\n\n";

    // Satisfiable hard instance: XOR constraints
    for (int num_vars : {20, 50, 100}) {
        std::vector<std::vector<int>> clauses;

        // XOR 3-SAT: (x1 XOR x2 XOR x3) = satisfiable
        std::mt19937 rng(999);
        std::vector<bool> known(num_vars);
        for (int i = 0; i < num_vars; i++) known[i] = rng() % 2;

        int num_clauses = num_vars * 4;
        for (int c = 0; c < num_clauses; c++) {
            int v1 = rng() % num_vars + 1;
            int v2 = rng() % num_vars + 1;
            int v3 = rng() % num_vars + 1;

            // I-set para maging satisfiable
            bool val1 = known[v1-1];
            bool val2 = known[v2-1];
            bool val3 = known[v3-1];

            // XOR parity = 0 (satisfiable)
            bool parity = val1 ^ val2 ^ val3;

            int lit1 = val1 ? v1 : -v1;
            int lit2 = val2 ? v2 : -v2;
            int lit3 = val3 ? v3 : -v3;

            // Kung parity = 0, gamitin ang (l1 OR l2 OR l3)
            // Kung parity = 1, gamitin ang (l1 OR l2 OR NOT l3)
            if (parity) lit3 = -lit3;

            clauses.push_back({lit1, lit2, lit3});
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        auto result = GoldenSAT::GoldenSATSolver::solve(clauses, num_vars);
        auto t1 = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(t1 - t0).count();

        std::cout << "Vars: " << num_vars
                  << " Clauses: " << num_clauses
                  << " Sat: " << result.satisfiable
                  << " Time: " << secs << "s\n";
    }

    return 0;
}
