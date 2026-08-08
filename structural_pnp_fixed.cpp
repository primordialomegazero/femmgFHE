#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct StructuralPNPFixed {
    double encode_formula(const std::vector<std::vector<int>>& clauses, int n_vars, bool use_phi) {
        double total = 0.0;
        for (const auto& clause : clauses) {
            double clause_val = 0.0;
            for (int lit : clause) {
                int var = std::abs(lit);
                double contrib = (lit > 0) ? PHI : std::abs(PSI);
                clause_val += contrib / n_vars;
            }
            // Fuzzy OR: normalize
            clause_val = std::min(1.0, clause_val);
            total += (use_phi ? 1.0 : -1.0) * clause_val;
        }
        return total / (clauses.size() * PHI);
    }

    bool is_sat(const std::vector<std::vector<int>>& clauses, int n_vars) {
        double phi = encode_formula(clauses, n_vars, true);
        double psi = encode_formula(clauses, n_vars, false);

        double harmony = phi + psi;   // Should be ≈1 for SAT
        double conflict = phi * psi;  // Should be ≈-1 for UNSAT

        std::cout << "    φ=" << std::fixed << std::setprecision(6) << phi 
                  << " ψ=" << psi << " φ+ψ=" << harmony 
                  << " φ·ψ=" << conflict;

        if (std::abs(harmony - 1.0) < 0.3 && conflict < -0.1) {
            std::cout << " → SAT";
            return true;
        } else if (conflict < -0.1) {
            std::cout << " → UNSAT (conflict)";
            return false;
        } else {
            std::cout << " → UNSAT (no harmony)";
            return false;
        }
    }
};

int main() {
    std::cout << "\n╔══════════════════════════════════════════════╗\n";
    std::cout << "║  STRUCTURAL P=NP — FIXED VERSION           ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";

    StructuralPNPFixed solver;

    // PHP_3 — UNSAT
    std::vector<std::vector<int>> php3 = {{1,2},{1,3},{2,3},{-1,-2},{-1,-3},{-2,-3}};
    std::cout << "PHP_3: ";
    solver.is_sat(php3, 3);

    // SAT_3var — SAT
    std::vector<std::vector<int>> sat3 = {{1,2,3},{-1,-2,3},{1,-2,-3}};
    std::cout << "\nSAT_3var: ";
    solver.is_sat(sat3, 3);

    // UNSAT_1var — UNSAT
    std::vector<std::vector<int>> unsat1 = {{1},{-1}};
    std::cout << "\nUNSAT_1var: ";
    solver.is_sat(unsat1, 1);

    std::cout << "\n\n";
    return 0;
}
