#include <iostream>
#include <vector>
#include <cmath>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

// ACTUAL fuzzy NAND from Theorem 20
double fuzzy_nand(double a, double b) {
    double ca = std::min(1.0, std::max(0.0, a));
    double cb = std::min(1.0, std::max(0.0, b));
    return 1.0 - ca * cb;
}

// ACTUAL clause evaluator
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

// ACTUAL full evaluator from Theorem 20
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
    // FORMULA: (x) AND (NOT x) — IMPOSSIBLE, MUST BE UNSAT
    std::vector<std::vector<int>> clauses = {{1}, {-1}};
    int n_vars = 1;
    
    double phi_val = std::abs(PHI) / (std::abs(PHI) + std::abs(PSI)); // 0.723607
    double psi_val = std::abs(PSI) / (std::abs(PHI) + std::abs(PSI)); // 0.276393
    
    std::vector<double> phi_vars(n_vars, phi_val);
    std::vector<double> psi_vars(n_vars, psi_val);
    
    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║  ACTUAL THEOREM 20 SOLVER — (x) AND (NOT x)       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "  Formula: (x) AND (NOT x)\n";
    std::cout << "  Truth:   UNSAT (impossible)\n\n";
    
    // φ-path
    std::cout << "  φ-path (var = " << phi_val << "):\n";
    double c1_phi = eval_clause(phi_vars, clauses[0]);
    double c2_phi = eval_clause(phi_vars, clauses[1]);
    double nand1 = fuzzy_nand(c1_phi, c2_phi);
    double result_phi = fuzzy_nand(nand1, nand1);
    std::cout << "    (x) = " << c1_phi << "\n";
    std::cout << "    (NOT x) = " << c2_phi << "\n";
    std::cout << "    NAND(x, NOT x) = " << nand1 << "\n";
    std::cout << "    NAND(NAND, NAND) = " << result_phi << "\n\n";
    
    // ψ-path
    std::cout << "  ψ-path (var = " << psi_val << "):\n";
    double c1_psi = eval_clause(psi_vars, clauses[0]);
    double c2_psi = eval_clause(psi_vars, clauses[1]);
    double nand2 = fuzzy_nand(c1_psi, c2_psi);
    double result_psi = fuzzy_nand(nand2, nand2);
    std::cout << "    (x) = " << c1_psi << "\n";
    std::cout << "    (NOT x) = " << c2_psi << "\n";
    std::cout << "    NAND(x, NOT x) = " << nand2 << "\n";
    std::cout << "    NAND(NAND, NAND) = " << result_psi << "\n\n";
    
    // Average (emergent SAT detection)
    double avg = (result_phi + result_psi) / 2.0;
    bool sat = avg > 0.5;
    
    std::cout << "  ─────────────────────────────────────────────\n";
    std::cout << "  φ-path result: " << result_phi << "\n";
    std::cout << "  ψ-path result: " << result_psi << "\n";
    std::cout << "  Average: " << avg << "\n";
    std::cout << "  Threshold: 0.5\n";
    std::cout << "  Verdict: " << (sat ? "SAT" : "UNSAT") << "\n";
    std::cout << "  Expected: UNSAT\n";
    std::cout << "  Match: " << (!sat ? "✅ CORRECT!" : "❌ WRONG") << "\n\n";
    
    // BONUS: Test (x) AND (NOT x) AND (x) — mas maraming clauses
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║  BONUS: (x) AND (NOT x) AND (x)                   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n\n";
    
    std::vector<std::vector<int>> clauses3 = {{1}, {-1}, {1}};
    double r_phi_3 = eval_all(phi_vars, clauses3);
    double r_psi_3 = eval_all(psi_vars, clauses3);
    double avg3 = (r_phi_3 + r_psi_3) / 2.0;
    
    std::cout << "  φ-path: " << r_phi_3 << "\n";
    std::cout << "  ψ-path: " << r_psi_3 << "\n";
    std::cout << "  Average: " << avg3 << "\n";
    std::cout << "  Verdict: " << (avg3 > 0.5 ? "SAT" : "UNSAT") << "\n";
    std::cout << "  Expected: UNSAT (may NOT x pa rin)\n\n";
    
    return 0;
}
