#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

struct MirrorObserver {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    std::vector<double> observer;
    std::vector<double> observed;
    long long decisions;
    
    MirrorObserver(int n, const std::vector<std::vector<int>>& cls)
        : n_vars(n), clauses(cls), decisions(0) {
        assignment.resize(n_vars + 1, 0);
        observer.resize(n_vars + 1, PHI);
        observed.resize(n_vars + 1, PSI);
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
    
    void mirror_update(int var, int value) {
        double obs_factor = (value == 1) ? PHI : PSI;
        double obd_factor = (value == 1) ? PSI : PHI;
        
        observer[var] *= obs_factor;
        observed[var] *= obd_factor;
        
        double sum = observer[var] + observed[var];
        if (fabs(sum) > 1e-12) {
            observer[var] /= sum;
            observed[var] /= sum;
        }
        
        double total_obs = 0, total_obd = 0, total_weight = 0;
        for (int u = 1; u <= n_vars; u++) {
            if (assignment[u] != 0 && u != var) {
                double dist = fabs(var - u);
                double weight = 1.0 / (1.0 + dist * dist / PHI);
                total_obs += observer[u] * weight;
                total_obd += observed[u] * weight;
                total_weight += weight;
            }
        }
        if (total_weight > 0) {
            double self_w = PHI;
            observer[var] = (observer[var] * self_w + total_obs) / (self_w + total_weight);
            observed[var] = (observed[var] * self_w + total_obd) / (self_w + total_weight);
            double ns = observer[var] + observed[var];
            if (fabs(ns) > 1e-12) {
                observer[var] /= ns;
                observed[var] /= ns;
            }
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
                    mirror_update(v, val);
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
            double score = observer[v] * PHI + observed[v] * PSI;
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
        double polarity = observer[v] + observed[v];
        int first_val = (polarity >= 0) ? 1 : -1;
        assignment[v] = first_val; mirror_update(v, first_val);
        if (dpll()) return true;
        assignment[v] = -first_val; mirror_update(v, -first_val);
        if (dpll()) return true;
        assignment[v] = 0;
        return false;
    }
    
    bool solve() {
        fill(assignment.begin(), assignment.end(), 0);
        for (int v = 1; v <= n_vars; v++) {
            observer[v] = PHI;
            observed[v] = PSI;
        }
        decisions = 0;
        return dpll();
    }
};

double sublinear_formula(double n) {
    return (1.0 / PHI) * pow(n, 1.0 / PHI);
}

int main() {
    std::vector<std::vector<int>> php3 = {
        {1,2}, {1,3}, {2,3}, {-1,-2}, {-1,-3}, {-2,-3}
    };
    MirrorObserver solver(3, php3);
    bool result = solver.solve();
    
    std::cout << "PHP_3: " << (result ? "SAT" : "UNSAT") 
              << " | decisions=" << solver.decisions 
              << " | S(3)=" << sublinear_formula(3) << "\n";
    
    return 0;
}
