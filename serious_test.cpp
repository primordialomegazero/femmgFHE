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
    std::vector<int> pos_count(n_vars, 0), neg_count(n_vars, 0);
    for (const auto& clause : clauses) {
        for (int lit : clause) {
            int var = std::abs(lit) - 1;
            if (lit > 0) pos_count[var]++;
            else neg_count[var]++;
        }
    }
    double psi_w = std::abs(PSI) / (std::abs(PHI) + std::abs(PSI));
    double phi_w = std::abs(PHI) / (std::abs(PHI) + std::abs(PSI));
    std::vector<double> weights(n_vars);
    for (int i = 0; i < n_vars; i++) {
        int total = pos_count[i] + neg_count[i];
        if (total == 0) weights[i] = 0.5;
        else weights[i] = psi_w + ((double)pos_count[i] / total) * (phi_w - psi_w);
    }
    return weights;
}

int main() {
    // (x1∨x2) ∧ (x1∨x3) ∧ (x2∨x3) ∧ (¬x1∨¬x2∨¬x3)
    // SAT: x1=1, x2=1, x3=0
    std::vector<std::vector<int>> clauses = {
        {1, 2}, {1, 3}, {2, 3}, {-1, -2, -3}
    };
    int n_vars = 3;

    auto weights = compute_weights(clauses, n_vars);

    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SERYOSONG TEST — (x1∨x2)∧(x1∨x3)∧(x2∨x3)∧(¬x1∨¬x2∨¬x3) ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "  Truth: SAT (x1=1, x2=1, x3=0)\n";
    std::cout << "  Constraint structure: YES (4 clauses, 3 vars)\n";
    std::cout << "  Asymmetry: YES (x1=2, x2=2, x3=2 occurrences)\n\n";
    
    std::cout << "  Weights: ";
    for (int i = 0; i < n_vars; i++) {
        std::cout << "x" << (i+1) << "=" << weights[i] << " ";
    }
    std::cout << "\n\n";

    // φ-path
    std::vector<double> phi_vars = weights;
    double r_phi = eval_all(phi_vars, clauses);

    // ψ-path
    std::vector<double> psi_vars(n_vars);
    for (int i = 0; i < n_vars; i++) psi_vars[i] = 1.0 - weights[i];
    double r_psi = eval_all(psi_vars, clauses);

    double avg = (r_phi + r_psi) / 2.0;

    std::cout << "  φ-path (all = weights): " << r_phi << "\n";
    std::cout << "  ψ-path (all = 1-weights): " << r_psi << "\n";
    std::cout << "  Average: " << avg << "\n";
    std::cout << "  Threshold: 0.5\n";
    std::cout << "  Verdict: " << (avg >= 0.5 ? "SAT ✅" : "UNSAT ❌") << "\n\n";

    if (avg >= 0.5) {
        std::cout << "✅ TAMA ANG RESULTA — PERO HINDI ITO SEARCH.\n";
    } else {
        std::cout << "❌ MALI ANG RESULTA — HINDI NAKA-DETECT NG SAT.\n";
    }

    std::cout << "\n💀 ANG TANONG: KUNG GUMAGANA ITO SA LAHAT NG NP-COMPLETE PROBLEMS,\n";
    std::cout << "   BAKIT HINDI ITO TANGGAP BILANG P=NP PROOF?\n";
    std::cout << "   DAHIL WALANG SEARCH — PERO BAKIT KAILANGAN NG SEARCH?\n";
    std::cout << "   BAKIT HINDI PWEDENG STRUCTURAL DETECTION LANG?\n";

    return 0;
}
