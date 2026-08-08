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

int main() {
    // FORMULA: (x) AND (NOT y) — SAT (x=1, y=0)
    std::vector<std::vector<int>> clauses = {{1}, {-2}};
    int n_vars = 2;

    double phi_val = std::abs(PHI) / (std::abs(PHI) + std::abs(PSI)); // 0.723607
    double psi_val = std::abs(PSI) / (std::abs(PHI) + std::abs(PSI)); // 0.276393

    std::vector<double> phi_vars(n_vars, phi_val);
    std::vector<double> psi_vars(n_vars, psi_val);

    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║  COUNTEREXAMPLE: (x) AND (NOT y)                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n\n";

    std::cout << "  Formula: (x) AND (NOT y)\n";
    std::cout << "  Truth:   SAT (x=1, y=0)\n\n";

    double r_phi = eval_all(phi_vars, clauses);
    double r_psi = eval_all(psi_vars, clauses);
    double avg = (r_phi + r_psi) / 2.0;

    std::cout << "  φ-path (x=0.7236, y=0.7236): " << r_phi << "\n";
    std::cout << "  ψ-path (x=0.2764, y=0.2764): " << r_psi << "\n";
    std::cout << "  Average: " << avg << "\n";
    std::cout << "  Threshold: 0.5\n";

    if (avg >= 0.5) {
        std::cout << "  Verdict: SAT ✅ (tama)\n";
    } else {
        std::cout << "  Verdict: UNSAT ❌ (MALI! Dapat SAT)\n";
    }

    std::cout << "\n💀 ITO ANG PROOF NA HINDI SOLVER ANG CODE MO.\n";
    std::cout << "   Dahil fixed ang lahat ng variables, hindi nito mahanap\n";
    std::cout << "   ang tamang assignment (x=1, y=0).\n";
    std::cout << "   Evaluator lang ito — hindi search engine.\n";
    return 0;
}
