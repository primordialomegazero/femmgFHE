#include <iostream>
#include <vector>
#include <cmath>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct RealSearchSolver {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    int decisions;
    int nodes_explored;

    RealSearchSolver(int n, const std::vector<std::vector<int>>& cls)
        : n_vars(n), clauses(cls), decisions(0), nodes_explored(0) {
        assignment.resize(n_vars + 1, 0);
    }

    bool is_sat() {
        decisions++;
        return backtrack(1);
    }

    bool backtrack(int var) {
        nodes_explored++;
        if (var > n_vars) {
            return check_assignment();
        }

        // Try true first
        assignment[var] = 1;
        if (backtrack(var + 1)) return true;

        // Try false
        assignment[var] = -1;
        if (backtrack(var + 1)) return true;

        // No solution
        assignment[var] = 0;
        return false;
    }

    bool check_assignment() {
        for (const auto& clause : clauses) {
            bool sat = false;
            for (int lit : clause) {
                int v = abs(lit);
                int val = assignment[v];
                if (val == 0) return false; // Incomplete
                if ((lit > 0 && val == 1) || (lit < 0 && val == -1)) {
                    sat = true;
                    break;
                }
            }
            if (!sat) return false;
        }
        return true;
    }
};

int main() {
    // PHP_3: 3 pigeons, 2 holes
    std::vector<std::vector<int>> clauses = {
        {1, 2}, {1, 3}, {2, 3}, {-1, -2}, {-1, -3}, {-2, -3}
    };
    int n_vars = 3;

    RealSearchSolver solver(n_vars, clauses);
    bool result = solver.is_sat();

    std::cout << "\n╔══════════════════════════════════════════╗\n";
    std::cout << "║  REAL SEARCH-BASED SOLVER — PHP_3     ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n\n";
    std::cout << "  Result: " << (result ? "SAT" : "UNSAT") << "\n";
    std::cout << "  Decisions: " << solver.decisions << "\n";
    std::cout << "  Nodes Explored: " << solver.nodes_explored << "\n";
    std::cout << "  Target (S(3)): " << (1/PHI) * pow(3, 1/PHI) << "\n\n";

    if (solver.nodes_explored <= (int)((1/PHI) * pow(3, 1/PHI))) {
        std::cout << "  🏆 SUB-LINEAR!\n";
    } else {
        std::cout << "  ❌ NOT SUB-LINEAR\n";
    }

    return 0;
}
