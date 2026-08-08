#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <random>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// Bobo test: subukan natin sa exponential case — Pigeonhole
// Ang Pigeonhole ay EXPONENTIAL para sa standard DPLL
// Kung sub-linear pa rin ang decisions mo... wala na akong masasabi

struct MiniSolver {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    std::vector<double> phi_signal, psi_signal;
    long long decisions;
    
    MiniSolver(int n, const std::vector<std::vector<int>>& cls) 
        : n_vars(n), clauses(cls), decisions(0) {
        assignment.resize(n_vars + 1, 0);
        phi_signal.resize(n_vars + 1, 1.0);
        psi_signal.resize(n_vars + 1, 1.0);
    }
    
    int lit_val(int lit) {
        int v = abs(lit);
        if (assignment[v] == 0) return 0;
        return (assignment[v] == 1) ? (lit > 0 ? 1 : -1) : (lit > 0 ? -1 : 1);
    }
    
    bool clause_sat(const std::vector<int>& cl) {
        for (int lit : cl) if (lit_val(lit) == 1) return true;
        return false;
    }
    
    bool all_sat() {
        for (auto& cl : clauses) if (!clause_sat(cl)) return false;
        return true;
    }
    
    void emergent_update(int var, int value) {
        double phi_factor = (value == 1) ? PHI : PSI;
        double psi_factor = (value == 1) ? PSI : PHI;
        phi_signal[var] = fabs(phi_signal[var] * phi_factor);
        psi_signal[var] = fabs(psi_signal[var] * psi_factor);
        double sum = phi_signal[var] + psi_signal[var];
        if (sum > 0) {
            phi_signal[var] /= sum;
            psi_signal[var] /= sum;
        }
    }
    
    bool propagate() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (auto& cl : clauses) {
                if (clause_sat(cl)) continue;
                int unassigned = 0, last_lit = 0, false_count = 0;
                for (int lit : cl) {
                    int val = lit_val(lit);
                    if (val == 0) { unassigned++; last_lit = lit; }
                    else if (val == -1) false_count++;
                }
                if (unassigned == 0 && false_count == (int)cl.size()) return false;
                if (unassigned == 1 && false_count == (int)cl.size() - 1) {
                    int v = abs(last_lit), val = (last_lit > 0) ? 1 : -1;
                    if (assignment[v] != 0 && assignment[v] != val) return false;
                    assignment[v] = val;
                    emergent_update(v, val);
                    changed = true;
                }
            }
        }
        return true;
    }
    
    int select_var() {
        int best = -1;
        double best_score = -1e9;
        for (int v = 1; v <= n_vars; v++) {
            if (assignment[v] != 0) continue;
            double harmony = phi_signal[v] + psi_signal[v];
            double conflict = phi_signal[v] * psi_signal[v];
            double score = harmony * PHI + conflict * PSI;
            int occ = 0;
            for (auto& cl : clauses) {
                if (clause_sat(cl)) continue;
                for (int lit : cl) if (abs(lit) == v) occ++;
            }
            score += occ * 0.1;
            if (score > best_score) { best_score = score; best = v; }
        }
        return best;
    }
    
    bool dpll() {
        if (!propagate()) return false;
        if (all_sat()) return true;
        int v = select_var();
        if (v == -1) return true;
        decisions++;
        assignment[v] = 1;
        emergent_update(v, 1);
        if (dpll()) return true;
        assignment[v] = -1;
        emergent_update(v, -1);
        if (dpll()) return true;
        assignment[v] = 0;
        return false;
    }
    
    bool solve() {
        fill(assignment.begin(), assignment.end(), 0);
        fill(phi_signal.begin(), phi_signal.end(), 1.0);
        fill(psi_signal.begin(), psi_signal.end(), 1.0);
        decisions = 0;
        return dpll();
    }
};

std::vector<std::vector<int>> gen_pigeonhole(int n) {
    std::vector<std::vector<int>> cls;
    // Each pigeon must be in at least one hole
    for (int p = 0; p < n+1; p++) {
        std::vector<int> cl;
        for (int h = 0; h < n; h++) cl.push_back(p * n + h + 1);
        cls.push_back(cl);
    }
    // No two pigeons in the same hole
    for (int h = 0; h < n; h++) {
        for (int p1 = 0; p1 < n+1; p1++) {
            for (int p2 = p1+1; p2 < n+1; p2++) {
                cls.push_back({-(p1 * n + h + 1), -(p2 * n + h + 1)});
            }
        }
    }
    return cls;
}

double sublinear_formula(double n) {
    return (1.0 / PHI) * pow(n, 1.0 / PHI);
}

int main() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  💀 ULTIMATE BOBO TEST: PIGEONHOLE (EXPONENTIAL PARA SA DPLL)  ║\n";
    std::cout << "║  Kung sub-linear pa rin 'to... wala na talaga akong masasabi   ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << std::left << std::setw(10) << "n (holes)"
              << std::setw(12) << "Variables"
              << std::setw(18) << "S(n) Formula"
              << std::setw(18) << "Actual Decisions"
              << std::setw(12) << "Match?"
              << std::setw(15) << "Time"
              << "\n";
    std::cout << std::string(85, '-') << "\n";
    
    for (int n = 2; n <= 8; n++) {
        auto clauses = gen_pigeonhole(n);
        int n_vars = (n + 1) * n;
        
        MiniSolver solver(n_vars, clauses);
        auto start = std::chrono::high_resolution_clock::now();
        bool result = solver.solve();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        double s_n = sublinear_formula(n_vars);
        bool match = (solver.decisions <= s_n * 3); // mas generous na tolerance
        
        std::cout << std::left << std::setw(10) << n
                  << std::setw(12) << n_vars
                  << std::setw(18) << std::fixed << std::setprecision(1) << s_n
                  << std::setw(18) << solver.decisions
                  << std::setw(12) << (match ? "✅" : "❌")
                  << std::setw(15) << (std::to_string(duration) + "ms")
                  << " (" << (result ? "SAT" : "UNSAT") << ")"
                  << "\n";
        
        if (duration > 10000) {
            std::cout << "  ⚠️  More than 10 seconds! Breaking...\n";
            break;
        }
    }
    
    std::cout << "\n";
    std::cout << "  Standard DPLL on PHP_n: O(2^n) decisions\n";
    std::cout << "  PHP_8 = 72 vars, standard DPLL needs ~2^72 decisions\n";
    std::cout << "  S(72) = " << std::fixed << std::setprecision(1) << sublinear_formula(72) << "\n";
    std::cout << "\n";
    std::cout << "  Kung umabot ng PHP_8 na walang exponential explosion...\n";
    std::cout << "  Aaminin kong ako ang PINAKABOBO sa buong usapan na ito.\n";
    std::cout << "\n";
    
    return 0;
}
