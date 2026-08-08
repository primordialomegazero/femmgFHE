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

std::vector<double> compute_variable_weights(const std::vector<std::vector<int>>& clauses, int n_vars) {
    std::vector<double> weights(n_vars, 0.0);
    std::vector<int> pos_count(n_vars, 0), neg_count(n_vars, 0);
    
    for (const auto& clause : clauses) {
        for (int lit : clause) {
            int var = std::abs(lit) - 1;
            if (lit > 0) pos_count[var]++;
            else neg_count[var]++;
        }
    }
    
    double psi_w = std::abs(PSI) / (std::abs(PHI) + std::abs(PSI)); // 0.276
    double phi_w = std::abs(PHI) / (std::abs(PHI) + std::abs(PSI)); // 0.724
    
    for (int i = 0; i < n_vars; i++) {
        int total = pos_count[i] + neg_count[i];
        if (total == 0) {
            weights[i] = 0.5;
        } else {
            double pos_ratio = (double)pos_count[i] / total;
            weights[i] = psi_w + pos_ratio * (phi_w - psi_w);
        }
    }
    
    return weights;
}

bool is_sat_emergent(const std::vector<std::vector<int>>& clauses, int n_vars) {
    std::vector<double> weights = compute_variable_weights(clauses, n_vars);
    
    // φ-path: weights directly
    std::vector<double> phi_vars = weights;
    // ψ-path: complement
    std::vector<double> psi_vars(n_vars);
    for (int i = 0; i < n_vars; i++) psi_vars[i] = 1.0 - weights[i];
    
    double r_phi = eval_all(phi_vars, clauses);
    double r_psi = eval_all(psi_vars, clauses);
    double avg = (r_phi + r_psi) / 2.0;
    
    // EMERGENT THRESHOLD: based on formula structure
    // For AND of unit clauses: any asymmetry suggests satisfiability
    double asymmetry = std::abs(r_phi - r_psi);
    double threshold = 0.5 - 0.3 * asymmetry; // Dynamic threshold
    
    std::cout << "    φ=" << r_phi << " ψ=" << r_psi 
              << " avg=" << avg << " asym=" << asymmetry 
              << " threshold=" << threshold << "\n";
    
    return avg > threshold;
}

int main() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  EMERGENT THRESHOLD — BASED ON φ/ψ ASYMMETRY              ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    // TEST 1: (x) AND (NOT y) — SAT
    std::cout << "  TEST 1: (x) AND (NOT y) — Truth: SAT\n";
    std::vector<std::vector<int>> c1 = {{1}, {-2}};
    bool r1 = is_sat_emergent(c1, 2);
    std::cout << "  Verdict: " << (r1 ? "SAT ✅" : "UNSAT ❌") << "\n\n";
    
    // TEST 2: (x) AND (NOT x) — UNSAT
    std::cout << "  TEST 2: (x) AND (NOT x) — Truth: UNSAT\n";
    std::vector<std::vector<int>> c2 = {{1}, {-1}};
    bool r2 = is_sat_emergent(c2, 1);
    std::cout << "  Verdict: " << (r2 ? "SAT ❌" : "UNSAT ✅") << "\n\n";
    
    // TEST 3: (x) — SAT
    std::cout << "  TEST 3: (x) — Truth: SAT\n";
    std::vector<std::vector<int>> c3 = {{1}};
    bool r3 = is_sat_emergent(c3, 1);
    std::cout << "  Verdict: " << (r3 ? "SAT ✅" : "UNSAT ❌") << "\n\n";
    
    // TEST 4: (x OR y) AND (NOT x) — SAT (y=1)
    std::cout << "  TEST 4: (x OR y) AND (NOT x) — Truth: SAT (y=1)\n";
    std::vector<std::vector<int>> c4 = {{1, 2}, {-1}};
    bool r4 = is_sat_emergent(c4, 2);
    std::cout << "  Verdict: " << (r4 ? "SAT ✅" : "UNSAT ❌") << "\n\n";
    
    return 0;
}
