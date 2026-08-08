#include <iostream>
#include <vector>
#include <cmath>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

double fuzzy_nand(double a, double b) {
    double ca = std::min(1.0, std::max(0.0, a));
    double cb = std::min(1.0, std::max(0.0, b));
    return 1.0 - ca * cb;
}

double eval_clause(const std::vector<double>& vars, const std::vector<int>& clause) {
    double result = 0.0;
    for (int lit : clause) {
        int var_idx = std::abs(lit) - 1;
        double val = vars[var_idx];
        if (lit < 0) val = 1.0 - val;
        result = std::max(result, val);
    }
    return result;
}

double eval_all(const std::vector<double>& vars, const std::vector<std::vector<int>>& clauses) {
    if (clauses.empty()) return 1.0;
    double acc = eval_clause(vars, clauses[0]);
    for (size_t c = 1; c < clauses.size(); c++) {
        double cv = eval_clause(vars, clauses[c]);
        double nv = fuzzy_nand(acc, cv);
        acc = fuzzy_nand(nv, nv);
    }
    return acc;
}

std::vector<double> compute_weights(const std::vector<std::vector<int>>& clauses, int n_vars) {
    std::vector<double> weights(n_vars, 0.5);
    // Simplified: use clause count heuristic
    return weights;
}

int main() {
    // SAT formula: (x OR y) AND (NOT x OR NOT y) AND (x OR z) AND (NOT x OR NOT z)
    // SAT assignment: x=1, y=0, z=0
    std::vector<std::vector<int>> clauses = {
        {1, 2}, {-1, -2}, {1, 3}, {-1, -3}
    };
    int n_vars = 3;

    double phi_val = std::abs(PHI) / (std::abs(PHI) + std::abs(PSI));
    double psi_val = std::abs(PSI) / (std::abs(PHI) + std::abs(PSI));

    std::vector<double> phi_vars(n_vars, phi_val);
    std::vector<double> psi_vars(n_vars, psi_val);

    double r_phi = eval_all(phi_vars, clauses);
    double r_psi = eval_all(psi_vars, clauses);
    double avg = (r_phi + r_psi) / 2.0;

    std::cout << "\n╔══════════════════════════════════════════╗\n";
    std::cout << "║  ULTIMATE TEST: (x∨y)∧(¬x∨¬y)∧(x∨z)∧(¬x∨¬z) ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n\n";

    std::cout << "  Truth: SAT (x=1, y=0, z=0)\n\n";
    std::cout << "  φ-path (all vars = 0.7236): " << r_phi << "\n";
    std::cout << "  ψ-path (all vars = 0.2764): " << r_psi << "\n";
    std::cout << "  Average: " << avg << "\n";
    std::cout << "  Threshold: 0.5\n";

    if (avg >= 0.5) {
        std::cout << "  Verdict: SAT ✅\n";
    } else {
        std::cout << "  Verdict: UNSAT ❌ (MALI! Dapat SAT)\n";
    }

    std::cout << "\n💀 KAHIT ANONG WEIGHTED AVERAGE GAWIN MO,\n";
    std::cout << "   HINDI MO MAHAHANAP ANG TAMANG ASSIGNMENT\n";
    std::cout << "   DAHIL WALANG SEARCH NA NANGYAYARI.\n";
    return 0;
}
