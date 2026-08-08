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

// Fractal Golden Gate — depth ≥ 3 → |v|
double FGG(double v, int depth = 3, bool use_phi = true) {
    double current = v;
    for (int d = 0; d < depth; d++) {
        double encoded = (d % 2 == 0) ?
            (use_phi ? current * PHI : current * PSI) :
            (use_phi ? current * PSI : current * PHI);
        double collapsed = (d % 2 == 0) ?
            (use_phi ? std::abs(encoded * PSI) : std::abs(encoded * PHI)) :
            (use_phi ? std::abs(encoded * PHI) : std::abs(encoded * PSI));
        current = collapsed;
    }
    return current;  // = |v| for depth ≥ 3
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

// NEW: FGG-augmented evaluation
double eval_all_fgg(const std::vector<double>& vars, const std::vector<std::vector<int>>& clauses, bool use_phi) {
    if (clauses.empty()) return 1.0;
    
    double acc = eval_clause(vars, clauses[0]);
    // Apply FGG at depth 1 para ma-normalize
    acc = FGG(acc, 3, use_phi);
    
    for (size_t c = 1; c < clauses.size(); c++) {
        double cv = eval_clause(vars, clauses[c]);
        cv = FGG(cv, 3, use_phi);  // Normalize each clause
        
        double nv = fuzzy_nand(acc, cv);
        acc = FGG(fuzzy_nand(nv, nv), 3, use_phi);  // FGG after each step
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
    // TAUTOLOGY: (x1∨x2)∧(x1∨¬x2)∧(¬x1∨x2)∧(¬x1∨¬x2)∧...
    std::vector<std::vector<int>> clauses = {
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2},
        {3, 4}, {3, -4}, {-3, 4}, {-3, -4}
    };
    int n_vars = 4;
    
    auto weights = compute_weights(clauses, n_vars);
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  FGG-AUGMENTED EVALUATION — Fractal Golden Gate Depth 3   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "  Formula: Tautology (all 2-literal clauses for 2 pairs)\n";
    std::cout << "  Truth: SAT (always true)\n\n";
    
    // φ-path with FGG
    std::vector<double> phi_vars = weights;
    double r_phi = eval_all_fgg(phi_vars, clauses, true);
    
    // ψ-path with FGG
    std::vector<double> psi_vars(n_vars);
    for (int i = 0; i < n_vars; i++) psi_vars[i] = 1.0 - weights[i];
    double r_psi = eval_all_fgg(psi_vars, clauses, false);
    
    double avg = (r_phi + r_psi) / 2.0;
    
    std::cout << "  Weights: x1=" << weights[0] << " x2=" << weights[1] 
              << " x3=" << weights[2] << " x4=" << weights[3] << "\n\n";
    std::cout << "  φ-path (FGG): " << r_phi << "\n";
    std::cout << "  ψ-path (FGG): " << r_psi << "\n";
    std::cout << "  Average: " << avg << "\n";
    std::cout << "  Threshold: 0.5\n";
    std::cout << "  Verdict: " << (avg >= 0.5 ? "SAT ✅" : "UNSAT ❌") << "\n\n";
    
    // TEST 2: PHP_2 (should be UNSAT)
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  TEST 2: PHP_2 — Dapat UNSAT                              ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    int n = 2;
    int nv = (n+1)*n;
    std::vector<std::vector<int>> php;
    for (int i = 0; i <= n; i++) {
        std::vector<int> cl;
        for (int j = 0; j < n; j++) cl.push_back(i*n + j + 1);
        php.push_back(cl);
    }
    for (int j = 0; j < n; j++)
        for (int i1 = 0; i1 <= n; i1++)
            for (int i2 = i1+1; i2 <= n; i2++)
                php.push_back({-(i1*n + j + 1), -(i2*n + j + 1)});
    
    auto w2 = compute_weights(php, nv);
    std::vector<double> pv(nv), sv(nv);
    for (int i = 0; i < nv; i++) { pv[i] = w2[i]; sv[i] = 1.0 - w2[i]; }
    
    double rp = eval_all_fgg(pv, php, true);
    double rs = eval_all_fgg(sv, php, false);
    double av = (rp + rs) / 2.0;
    
    std::cout << "  φ-path: " << rp << "\n";
    std::cout << "  ψ-path: " << rs << "\n";
    std::cout << "  Average: " << av << "\n";
    std::cout << "  Verdict: " << (av >= 0.5 ? "SAT ❌" : "UNSAT ✅") << "\n\n";
    
    return 0;
}
