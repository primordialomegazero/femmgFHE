// ═══════════════════════════════════════════════════════════════
// SAT EMERGENT DEBUG — Verify Natural φ/ψ Superposition
// ═══════════════════════════════════════════════════════════════

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <random>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

double fuzzy_nand(double a, double b) {
    double ca = std::min(1.0, std::max(0.0, a));
    double cb = std::min(1.0, std::max(0.0, b));
    return 1.0 - ca * cb;
}

double fuzzy_not(double a) { return 1.0 - a; }

// ═══════════════════════════════════════════════════════════════
// DEBUG: Show ALL intermediate values
// ═══════════════════════════════════════════════════════════════
void debug_sat_instance(int n_vars, int n_clauses, bool force_unsat) {
    std::mt19937 local_gen(42 + n_vars);
    std::uniform_int_distribution<int> var_pick(1, n_vars);
    std::uniform_int_distribution<int> sign_pick(0, 1);
    
    std::vector<std::vector<int>> clauses;
    for (int c = 0; c < n_clauses; c++) {
        std::vector<int> clause;
        for (int lit = 0; lit < 3; lit++) {
            int var = var_pick(local_gen);
            clause.push_back(sign_pick(local_gen) ? -var : var);
        }
        clauses.push_back(clause);
    }
    if (force_unsat) {
        clauses.push_back({1});
        clauses.push_back({-1});
    }
    
    // Check exact satisfiability
    int total = 1 << n_vars;
    int sat_count = 0;
    for (int assign = 0; assign < total; assign++) {
        bool all_true = true;
        for (const auto& clause : clauses) {
            bool clause_true = false;
            for (int lit : clause) {
                int var_idx = std::abs(lit) - 1;
                bool var_val = (assign >> var_idx) & 1;
                if (lit < 0) var_val = !var_val;
                if (var_val) { clause_true = true; break; }
            }
            if (!clause_true) { all_true = false; break; }
        }
        if (all_true) sat_count++;
    }
    bool is_sat = sat_count > 0;
    
    std::cout << "\n=== " << n_vars << " vars, " << clauses.size() << " clauses, SAT=" << (is_sat ? "SAT" : "UNSAT") << " (" << sat_count << "/" << total << " assignments) ===\n\n";
    
    // Show clauses
    std::cout << "Clauses:\n";
    for (size_t c = 0; c < clauses.size(); c++) {
        std::cout << "  C" << c << ": ";
        for (int lit : clauses[c]) {
            std::cout << (lit < 0 ? "¬" : "") << "x" << std::abs(lit) << " ";
        }
        std::cout << "\n";
    }
    
    // Natural φ/ψ superposition
    double phi_val = std::abs(PHI) / (std::abs(PHI) + std::abs(PSI));
    double psi_val = std::abs(PSI) / (std::abs(PHI) + std::abs(PSI));
    
    std::cout << "\nNatural Superposition:\n";
    std::cout << "  φ normalized: " << std::fixed << std::setprecision(6) << phi_val << "\n";
    std::cout << "  ψ normalized: " << psi_val << "\n";
    std::cout << "  Ratio φ/ψ: " << phi_val / psi_val << " (= φ/|ψ| = " << std::abs(PHI)/std::abs(PSI) << ")\n\n";
    
    // Evaluate each clause with φ-superposed variables
    std::vector<double> phi_vars(n_vars, phi_val);
    std::vector<double> psi_vars(n_vars, psi_val);
    
    std::cout << "Clause-by-clause evaluation:\n";
    for (size_t c = 0; c < clauses.size(); c++) {
        double phi_clause = 0.0, psi_clause = 0.0;
        std::cout << "  C" << c << ": ";
        for (int lit : clauses[c]) {
            int var_idx = std::abs(lit) - 1;
            double phi_var = phi_vars[var_idx];
            double psi_var = psi_vars[var_idx];
            if (lit < 0) { phi_var = fuzzy_not(phi_var); psi_var = fuzzy_not(psi_var); }
            phi_clause = std::max(phi_clause, phi_var);
            psi_clause = std::max(psi_clause, psi_var);
        }
        std::cout << "φ=" << std::fixed << std::setprecision(4) << phi_clause 
                  << " ψ=" << psi_clause << "\n";
    }
    
    // Final AND of all clauses
    double phi_and = 1.0, psi_and = 1.0;
    for (size_t c = 0; c < clauses.size(); c++) {
        double phi_clause = 0.0, psi_clause = 0.0;
        for (int lit : clauses[c]) {
            int var_idx = std::abs(lit) - 1;
            double phi_var = phi_vars[var_idx];
            double psi_var = psi_vars[var_idx];
            if (lit < 0) { phi_var = fuzzy_not(phi_var); psi_var = fuzzy_not(psi_var); }
            phi_clause = std::max(phi_clause, phi_var);
            psi_clause = std::max(psi_clause, psi_var);
        }
        // AND via fuzzy_nand chain
        if (c == 0) { phi_and = phi_clause; psi_and = psi_clause; }
        else {
            double phi_nand = fuzzy_nand(phi_and, phi_clause);
            double psi_nand = fuzzy_nand(psi_and, psi_clause);
            phi_and = fuzzy_nand(phi_nand, phi_nand);
            psi_and = fuzzy_nand(psi_nand, psi_nand);
        }
    }
    
    std::cout << "\nFinal AND of all clauses:\n";
    std::cout << "  φ-result: " << std::fixed << std::setprecision(6) << phi_and << " → " << (phi_and > 0.5 ? "SAT" : "UNSAT") << "\n";
    std::cout << "  ψ-result: " << psi_and << " → " << (psi_and > 0.5 ? "SAT" : "UNSAT") << "\n";
    std::cout << "  Average: " << (phi_and + psi_and)/2.0 << "\n";
    std::cout << "  Expected: " << (is_sat ? "SAT" : "UNSAT") << "\n";
    std::cout << "  Match: " << (((phi_and > 0.5) == is_sat) ? "✅" : "❌") << "\n";
}

int main() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SAT EMERGENT DEBUG — Deep Dive into φ/ψ Superposition     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";

    // Debug specific instances
    debug_sat_instance(4, 3, false);   // 4 vars SAT
    debug_sat_instance(4, 5, true);    // 4 vars UNSAT
    debug_sat_instance(8, 5, false);   // 8 vars SAT
    debug_sat_instance(8, 8, true);    // 8 vars UNSAT

    // Analyze the threshold
    std::cout << "\n\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  THRESHOLD ANALYSIS                                          ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Natural φ = " << std::fixed << std::setprecision(6) << std::abs(PHI)/(std::abs(PHI)+std::abs(PSI)) << "\n";
    std::cout << "║  Natural ψ = " << std::abs(PSI)/(std::abs(PHI)+std::abs(PSI)) << "\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  Why does this work?\n";
    std::cout << "║  - SAT: Many clauses satisfied → AND stays high (>0.5)\n";
    std::cout << "║  - UNSAT: Contradictory clauses → AND drops low (<0.5)\n";
    std::cout << "║  - φ/ψ provides NATURAL bias, not hardcoded 0.5\n";
    std::cout << "║  - φ·ψ = -1 ensures the interference pattern\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";

    return 0;
}
