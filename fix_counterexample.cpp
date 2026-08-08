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

// ═══════════════════════════════════════════════════════════════
// FIX: Per-variable φ/ψ encoding based on occurrences
// ═══════════════════════════════════════════════════════════════
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
    
    // φ-weight: more positive = closer to φ, more negative = closer to ψ
    for (int i = 0; i < n_vars; i++) {
        int total = pos_count[i] + neg_count[i];
        if (total == 0) {
            weights[i] = 0.5; // neutral
        } else {
            // Ratio of positive occurrences
            double pos_ratio = (double)pos_count[i] / total;
            // Map to [|ψ|/sum, |φ|/sum] range
            double psi_w = std::abs(PSI) / (std::abs(PHI) + std::abs(PSI)); // 0.276
            double phi_w = std::abs(PHI) / (std::abs(PHI) + std::abs(PSI)); // 0.724
            weights[i] = psi_w + pos_ratio * (phi_w - psi_w);
        }
    }
    
    return weights;
}

int main() {
    // FORMULA: (x) AND (NOT y) — SAT (x=1, y=0)
    std::vector<std::vector<int>> clauses = {{1}, {-2}};
    int n_vars = 2;
    
    // OLD METHOD: Same value for all variables
    double phi_val = std::abs(PHI) / (std::abs(PHI) + std::abs(PSI));
    double psi_val = std::abs(PSI) / (std::abs(PHI) + std::abs(PSI));
    std::vector<double> old_phi(n_vars, phi_val);
    std::vector<double> old_psi(n_vars, psi_val);
    
    // NEW METHOD: Per-variable φ/ψ weights
    std::vector<double> weights = compute_variable_weights(clauses, n_vars);
    // φ-path: use weights directly
    // ψ-path: use 1 - weights (complement)
    std::vector<double> new_phi = weights;
    std::vector<double> new_psi(n_vars);
    for (int i = 0; i < n_vars; i++) new_psi[i] = 1.0 - weights[i];
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  FIX: PER-VARIABLE φ/ψ ENCODING                           ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "  Formula: (x) AND (NOT y)\n";
    std::cout << "  Truth:   SAT (x=1, y=0)\n\n";
    
    std::cout << "  OLD METHOD (same values):\n";
    double old_r_phi = eval_all(old_phi, clauses);
    double old_r_psi = eval_all(old_psi, clauses);
    double old_avg = (old_r_phi + old_r_psi) / 2.0;
    std::cout << "    φ-path: " << old_r_phi << "\n";
    std::cout << "    ψ-path: " << old_r_psi << "\n";
    std::cout << "    Average: " << old_avg << "\n";
    std::cout << "    Verdict: " << (old_avg > 0.5 ? "SAT" : "UNSAT") << " ❌\n\n";
    
    std::cout << "  NEW METHOD (per-variable weights):\n";
    std::cout << "    x occurrences: +1 (1 pos, 0 neg) → weight = PHI-heavy\n";
    std::cout << "    y occurrences: -1 (0 pos, 1 neg) → weight = PSI-heavy\n";
    std::cout << "    x_phi = " << new_phi[0] << ", y_phi = " << new_phi[1] << "\n";
    std::cout << "    x_psi = " << new_psi[0] << ", y_psi = " << new_psi[1] << "\n";
    
    double new_r_phi = eval_all(new_phi, clauses);
    double new_r_psi = eval_all(new_psi, clauses);
    double new_avg = (new_r_phi + new_r_psi) / 2.0;
    std::cout << "    φ-path: " << new_r_phi << "\n";
    std::cout << "    ψ-path: " << new_r_psi << "\n";
    std::cout << "    Average: " << new_avg << "\n";
    std::cout << "    Verdict: " << (new_avg > 0.5 ? "SAT ✅" : "UNSAT ❌") << "\n\n";
    
    return 0;
}
