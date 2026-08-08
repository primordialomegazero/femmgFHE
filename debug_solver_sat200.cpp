#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <set>
#include <random>
#include <algorithm>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

struct DebugSolver {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    std::vector<double> phi_signal, psi_signal;
    long long decisions, conflicts, propagations;
    bool verbose;
    
    DebugSolver(int n, const std::vector<std::vector<int>>& cls, bool v) 
        : n_vars(n), clauses(cls), decisions(0), conflicts(0), propagations(0), verbose(v) {
        assignment.resize(n_vars + 1, 0);
        phi_signal.resize(n_vars + 1, PHI);
        psi_signal.resize(n_vars + 1, PSI);
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
    
    void fractal_update(int var, int value) {
        double phi_factor = (value == 1) ? PHI : PSI;
        double psi_factor = (value == 1) ? PSI : PHI;
        phi_signal[var] *= phi_factor;
        psi_signal[var] *= psi_factor;
        double sum = phi_signal[var] + psi_signal[var];
        if (fabs(sum) > 1e-12) {
            phi_signal[var] /= sum;
            psi_signal[var] /= sum;
        }
        
        double total_phi = 0, total_psi = 0, total_weight = 0;
        for (int u = 1; u <= n_vars; u++) {
            if (assignment[u] != 0 && u != var) {
                double dist = fabs(var - u);
                double weight = 1.0 / (1.0 + dist * dist / PHI);
                total_phi += phi_signal[u] * weight;
                total_psi += psi_signal[u] * weight;
                total_weight += weight;
            }
        }
        
        if (total_weight > 0) {
            double self_w = PHI;
            phi_signal[var] = (phi_signal[var] * self_w + total_phi) / (self_w + total_weight);
            psi_signal[var] = (psi_signal[var] * self_w + total_psi) / (self_w + total_weight);
            double ns = phi_signal[var] + psi_signal[var];
            if (fabs(ns) > 1e-12) {
                phi_signal[var] /= ns;
                psi_signal[var] /= ns;
            }
        }
    }
    
    bool propagate() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (int ci = 0; ci < (int)clauses.size(); ci++) {
                auto& cl = clauses[ci];
                if (clause_sat(cl)) continue;
                
                int unassigned = 0, last_lit = 0, false_count = 0;
                for (int lit : cl) {
                    int val = lit_val(lit);
                    if (val == 0) { unassigned++; last_lit = lit; }
                    else if (val == -1) false_count++;
                }
                
                if (unassigned == 0 && false_count == (int)cl.size()) {
                    if (verbose) {
                        std::cout << "  CONFLICT sa clause " << ci << ": ";
                        for (int lit : cl) std::cout << lit << "(" << lit_val(lit) << ") ";
                        std::cout << "\n";
                    }
                    conflicts++;
                    return false;
                }
                
                if (unassigned == 1 && false_count == (int)cl.size() - 1) {
                    int v = abs(last_lit), val = (last_lit > 0) ? 1 : -1;
                    if (assignment[v] != 0 && assignment[v] != val) {
                        if (verbose) {
                            std::cout << "  CONFLICT sa clause " << ci << " (unit): var " << v 
                                      << " assigned " << assignment[v] << " need " << val << "\n";
                        }
                        conflicts++;
                        return false;
                    }
                    assignment[v] = val;
                    propagations++;
                    fractal_update(v, val);
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
            double score = phi_signal[v] * PHI + psi_signal[v] * PSI;
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
        double polarity = phi_signal[v] + psi_signal[v];
        int first_val = (polarity >= 0) ? 1 : -1;
        
        assignment[v] = first_val;
        fractal_update(v, first_val);
        if (dpll()) return true;
        
        assignment[v] = -first_val;
        fractal_update(v, -first_val);
        if (dpll()) return true;
        
        assignment[v] = 0;
        return false;
    }
    
    bool solve() {
        fill(assignment.begin(), assignment.end(), 0);
        for (int v = 1; v <= n_vars; v++) {
            phi_signal[v] = PHI;
            psi_signal[v] = PSI;
        }
        decisions = conflicts = propagations = 0;
        return dpll();
    }
};

std::vector<std::vector<int>> gen_guaranteed_sat_1plus1(int n_vars, int n_clauses, std::vector<int>& solution) {
    std::mt19937 rng(12345 + n_vars);
    std::uniform_int_distribution<int> var_dist(1, n_vars);
    std::uniform_int_distribution<int> sign_dist(0, 1);
    std::uniform_int_distribution<int> lit_count_dist(2, 5);
    
    solution.resize(n_vars + 1);
    for (int i = 1; i <= n_vars; i++) solution[i] = (sign_dist(rng) == 0) ? 1 : -1;
    
    std::vector<std::vector<int>> clauses;
    std::set<std::string> seen;
    
    for (int i = 0; i < n_clauses * 3 && (int)clauses.size() < n_clauses; i++) {
        int k = lit_count_dist(rng);
        std::set<int> used_vars;
        while ((int)used_vars.size() < k) used_vars.insert(var_dist(rng));
        std::vector<int> vars_vec(used_vars.begin(), used_vars.end());
        int satisfy_idx = rng() % vars_vec.size();
        std::vector<int> cl;
        std::string key;
        for (int j = 0; j < (int)vars_vec.size(); j++) {
            int v = vars_vec[j];
            int lit = (j == satisfy_idx) ? ((solution[v] == 1) ? v : -v) : ((sign_dist(rng) == 0) ? v : -v);
            cl.push_back(lit);
            key += std::to_string(lit) + ",";
        }
        shuffle(cl.begin(), cl.end(), rng);
        if (seen.find(key) == seen.end()) {
            clauses.push_back(cl);
            seen.insert(key);
        }
    }
    return clauses;
}

int main() {
    std::vector<int> solution;
    auto clauses = gen_guaranteed_sat_1plus1(200, 600, solution);
    
    std::cout << "Testing SAT_200 with verbose conflict detection...\n";
    std::cout << "Solution exists (verified).\n\n";
    
    DebugSolver solver(200, clauses, true);
    bool result = solver.solve();
    
    std::cout << "\nResult: " << (result ? "SAT" : "UNSAT") << "\n";
    std::cout << "Decisions: " << solver.decisions << "\n";
    std::cout << "Conflicts: " << solver.conflicts << "\n";
    std::cout << "Propagations: " << solver.propagations << "\n";
    
    return 0;
}
