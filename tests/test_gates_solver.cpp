#include "src/np/golden_exact_gates.h"
#include <iostream>

int main() {
    std::cout << "=== GOLDEN EXACT SOLVER — ALL GATES ===\n\n";

    // Test: bawat gate ay may katumbas na satisfiable at unsat
    auto test_gate = [](const std::string& gate, bool a, bool b) {
        // I-encode ang gate bilang clauses
        std::vector<std::vector<int>> clauses;
        // Para sa satisfiable: ang assignment ay dapat mag-satisfy
        // Para sa unsat: may contradiction

        int num_vars = 2;

        // Simpleng encoding ng gate bilang SAT
        if (gate == "NAND") {
            clauses.push_back({-1, -2}); // NAND(a,b) = 1 kung hindi parehong true
        } else if (gate == "AND") {
            clauses.push_back({1});  // a = true
            clauses.push_back({2});  // b = true
        } else if (gate == "OR") {
            clauses.push_back({1, 2}); // a OR b
        } else if (gate == "XOR") {
            clauses.push_back({1, -2});
            clauses.push_back({-1, 2});
        } else if (gate == "NOR") {
            clauses.push_back({-1});
            clauses.push_back({-2});
        } else if (gate == "XNOR") {
            // XNOR(a,b) = (a AND b) OR (NOT a AND NOT b)
            // = (a OR NOT b) AND (NOT a OR b)
            clauses.push_back({1, -2});
            clauses.push_back({-1, 2});
        }

        auto result = GoldenExactGates::GoldenExactSolver::solve(clauses, num_vars);
        std::cout << "  " << gate << "(" << a << "," << b << "): sat="
                  << result.satisfiable << "\n";
        if (result.satisfiable) {
            std::cout << "    Assignment: ";
            for (bool v : result.assignment) std::cout << v << " ";
            std::cout << "\n";
        }
    };

    std::cout << "--- GATE SET ---\n";
    test_gate("NAND", true, true);
    test_gate("AND", true, true);
    test_gate("OR", false, false);
    test_gate("XOR", true, false);
    test_gate("NOR", false, false);
    test_gate("XNOR", true, true);

    return 0;
}
