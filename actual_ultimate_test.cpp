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
// ACTUAL PER-VARIABLE WEIGHTS (hindi 0.5 lahat!)
// ═══════════════════════════════════════════════════════════════
std::vector<double> compute_variable_weights(const std::vector<std::vector<int>>& clauses, int n_vars) {
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
    
    std::vector<double> weights(n_vars);
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

int main() {
    // (x OR y) AND (NOT x OR NOT y) AND (x OR z) AND (NOT x OR NOT z)
    // Truth: SAT (x=1, y=0, z=0)
    std::vector<std::vector<int>> clauses = {
        {1, 2}, {-1, -2}, {1, 3}, {-1, -3}
    };
    int n_vars = 3;
    
    std::vector<double> weights = compute_variable_weights(clauses, n_vars);
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ACTUAL SOLVER — WITH PER-VARIABLE φ/ψ WEIGHTS            ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "  Formula: (x∨y) ∧ (¬x∨¬y) ∧ (x∨z) ∧ (¬x∨¬z)\n";
    std::cout << "  Truth: SAT (x=1, y=0, z=0)\n\n";
    
    std::cout << "  Variable occurrences:\n";
    std::cout << "    x: +{1,3}, -{2,4} → 2 pos, 2 neg → weight=0.5\n";
    std::cout << "    y: +{1}, -{2} → 1 pos, 1 neg → weight=0.5\n";
    std::cout << "    z: +{3}, -{4} → 1 pos, 1 neg → weight=0.5\n\n";
    
    std::cout << "  Weights: x=" << weights[0] << ", y=" << weights[1] 
              << ", z=" << weights[2] << "\n\n";
    
    // φ-path
    std::vector<double> phi_vars = weights;
    // ψ-path
    std::vector<double> psi_vars(n_vars);
    for (int i = 0; i < n_vars; i++) psi_vars[i] = 1.0 - weights[i];
    
    double r_phi = eval_all(phi_vars, clauses);
    double r_psi = eval_all(psi_vars, clauses);
    double avg = (r_phi + r_psi) / 2.0;
    
    std::cout << "  φ-path result: " << r_phi << "\n";
    std::cout << "  ψ-path result: " << r_psi << "\n";
    std::cout << "  Average: " << avg << "\n";
    
    // NOTE: Lahat ng weights ay 0.5 kasi balanced ang pos/neg
    // Ibig sabihin: UNIFORM φ/ψ superposition
    // For SAT with uniform weights: dapat may emergent signal
    
    // EMERGENT ANALYSIS:
    // φ+ψ=1 → SAT signal
    // φ·ψ=-1 → UNSAT signal
    double harmony = r_phi + r_psi;    // Dapat ≈1 for SAT
    double conflict = r_phi * r_psi;   // Dapat ≈-1 for UNSAT
    
    std::cout << "\n  Harmony (φ+ψ): " << harmony << " (ideal=1 for SAT)\n";
    std::cout << "  Conflict (φ·ψ): " << conflict << " (ideal=-1 for UNSAT)\n";
    
    // KUNG UNIFORM ANG WEIGHTS, ANG FORMULA AY SAT KUNG:
    // May independence sa variables (hindi contradictory unit clauses)
    // Ang (x∨y)∧(¬x∨¬y)∧(x∨z)∧(¬x∨¬z) ay SAT kasi x=1,y=0,z=0 works
    
    std::cout << "\n  ⚠️  LAHAT NG WEIGHTS AY 0.5 DAHIL BALANCED ANG POS/NEG\n";
    std::cout << "  ⚠️  ANG SOLVER AY HINDI NAKAKA-DETECT NG SAT KAPAG\n";
    std::cout << "  ⚠️  LAHAT NG VARIABLES AY MAY PANTAY NA POS/NEG\n";
    std::cout << "  ⚠️  KELANGAN NG ASYMMETRY PARA MAG-EMERGE ANG SIGNAL\n\n";
    
    return 0;
}
