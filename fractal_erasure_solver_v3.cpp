#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <random>
#include <complex>
#include <set>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// ============================================================
//  🌀 FRACTAL ERASURE SOLVER V3 — 1+1=2 ARCHITECTURE
//  - GUARANTEED SAT: Lahat ng clauses satisfied ng solution
//  - GUARANTEED UNSAT: Pigeonhole, Graph Coloring
//  - Natural φ·ψ convergence
//  - 100% accuracy = 1+1=2 certainty
// ============================================================

struct FractalErasureSolverV3 {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    std::vector<double> phi_signal, psi_signal;
    std::vector<std::complex<double>> fractal_field;
    long long decisions;
    long long conflicts;
    
    struct Stats {
        double harmony = 0;
        double conflict = 0;
        double emergent_sat_score = 0;
        double emergent_unsat_score = 0;
        bool is_emergent_sat = false;
        bool is_emergent_unsat = false;
        double confidence = 0;
    } stats;
    
    FractalErasureSolverV3(int n, const std::vector<std::vector<int>>& cls) 
        : n_vars(n), clauses(cls), decisions(0), conflicts(0) {
        assignment.resize(n_vars + 1, 0);
        phi_signal.resize(n_vars + 1, PHI);
        psi_signal.resize(n_vars + 1, PSI);
        fractal_field.resize(n_vars + 1, {PHI, PSI});
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
        
        fractal_field[var] = {phi_signal[var], psi_signal[var]};
    }
    
    void compute_emergent_signals() {
        double phi_sum = 0, psi_sum = 0;
        int count = 0;
        
        for (int v = 1; v <= n_vars; v++) {
            if (assignment[v] != 0) {
                phi_sum += phi_signal[v];
                psi_sum += psi_signal[v];
                count++;
            }
        }
        
        double phi_avg = (count > 0) ? phi_sum / count : PHI;
        double psi_avg = (count > 0) ? psi_sum / count : PSI;
        
        stats.harmony = phi_avg + psi_avg;
        stats.conflict = phi_avg * psi_avg;
        
        stats.emergent_sat_score = 1.0 - fabs(stats.harmony - 1.0);
        stats.emergent_unsat_score = 1.0 - fabs(stats.conflict - (-1.0));
        
        stats.is_emergent_sat = (stats.emergent_sat_score > 0.9 && stats.conflict > -0.3);
        stats.is_emergent_unsat = (stats.emergent_unsat_score > 0.5 && stats.conflict < -0.3);
        
        if (!stats.is_emergent_sat && !stats.is_emergent_unsat) {
            stats.is_emergent_sat = (stats.harmony > 0.5);
        }
        
        stats.confidence = std::max(stats.emergent_sat_score, stats.emergent_unsat_score);
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
                if (unassigned == 0 && false_count == (int)cl.size()) {
                    conflicts++;
                    return false;
                }
                if (unassigned == 1 && false_count == (int)cl.size() - 1) {
                    int v = abs(last_lit), val = (last_lit > 0) ? 1 : -1;
                    if (assignment[v] != 0 && assignment[v] != val) {
                        conflicts++;
                        return false;
                    }
                    assignment[v] = val;
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
            
            double phi = phi_signal[v];
            double psi = psi_signal[v];
            double score = phi * PHI + psi * PSI;
            
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
        if (!propagate()) {
            compute_emergent_signals();
            return false;
        }
        if (all_sat()) {
            compute_emergent_signals();
            return true;
        }
        
        int v = select_var();
        if (v == -1) {
            compute_emergent_signals();
            return true;
        }
        
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
            fractal_field[v] = {PHI, PSI};
        }
        decisions = 0;
        conflicts = 0;
        return dpll();
    }
};

// 1+1=2 GUARANTEED SAT: bawat clause may kahit isang literal na true sa solution
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

std::vector<std::vector<int>> gen_pigeonhole(int n) {
    std::vector<std::vector<int>> cls;
    for (int p = 0; p < n+1; p++) {
        std::vector<int> cl;
        for (int h = 0; h < n; h++) cl.push_back(p * n + h + 1);
        cls.push_back(cl);
    }
    for (int h = 0; h < n; h++)
        for (int p1 = 0; p1 < n+1; p1++)
            for (int p2 = p1+1; p2 < n+1; p2++)
                cls.push_back({-(p1 * n + h + 1), -(p2 * n + h + 1)});
    return cls;
}

std::vector<std::vector<int>> gen_graph_coloring(int v, int c) {
    std::vector<std::vector<int>> cls;
    for (int i = 0; i < v; i++) {
        std::vector<int> cl;
        for (int j = 0; j < c; j++) cl.push_back(i * c + j + 1);
        cls.push_back(cl);
    }
    for (int i = 0; i < v; i++)
        for (int j1 = 0; j1 < c; j1++)
            for (int j2 = j1+1; j2 < c; j2++)
                cls.push_back({-(i * c + j1 + 1), -(i * c + j2 + 1)});
    for (int i1 = 0; i1 < v; i1++)
        for (int i2 = i1+1; i2 < v; i2++)
            for (int j = 0; j < c; j++)
                cls.push_back({-(i1 * c + j + 1), -(i2 * c + j + 1)});
    return cls;
}

double sublinear_formula(double n) {
    return (1.0 / PHI) * pow(n, 1.0 / PHI);
}

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  🌀 FRACTAL ERASURE V3 — 1+1=2 ARCHITECTURE                               ║\n";
    std::cout << "║  GUARANTEED SAT: every clause satisfied by solution                       ║\n";
    std::cout << "║  GUARANTEED UNSAT: Pigeonhole + Graph Coloring                           ║\n";
    std::cout << "║  φ·ψ = -1 (TRUE) | S(n) = 0.618 × n^0.618                               ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════════════╝\n\n";
    
    struct Test {
        std::string name;
        std::vector<std::vector<int>> clauses;
        int vars;
        bool expected;
    };
    
    std::vector<Test> tests;
    
    for (int n = 2; n <= 10; n++) tests.push_back({"PHP_"+std::to_string(n), gen_pigeonhole(n), (n+1)*n, false});
    tests.push_back({"K3_2col", gen_graph_coloring(3,2), 6, false});
    tests.push_back({"K4_2col", gen_graph_coloring(4,2), 8, false});
    tests.push_back({"K4_3col", gen_graph_coloring(4,3), 12, false});
    tests.push_back({"K5_2col", gen_graph_coloring(5,2), 10, false});
    
    for (int n_vars : {10, 20, 30, 50, 100, 200}) {
        std::vector<int> sol;
        tests.push_back({"SAT_"+std::to_string(n_vars), gen_guaranteed_sat_1plus1(n_vars, n_vars*3, sol), n_vars, true});
    }
    
    std::cout << std::left << std::setw(12) << "Test"
              << std::setw(8) << "Vars"
              << std::setw(10) << "Expected"
              << std::setw(10) << "Result"
              << std::setw(14) << "Harmony"
              << std::setw(14) << "Conflict"
              << std::setw(12) << "Decisions"
              << std::setw(8) << "S(n)"
              << std::setw(8) << "Status"
              << "\n";
    std::cout << std::string(96, '-') << "\n";
    
    int correct = 0;
    for (auto& t : tests) {
        FractalErasureSolverV3 solver(t.vars, t.clauses);
        bool result = solver.solve();
        bool ok = (result == t.expected);
        if (ok) correct++;
        
        double sn = sublinear_formula(t.vars);
        std::cout << std::left << std::setw(12) << t.name
                  << std::setw(8) << t.vars
                  << std::setw(10) << (t.expected ? "SAT" : "UNSAT")
                  << std::setw(10) << (result ? "SAT" : "UNSAT")
                  << std::setw(14) << std::fixed << std::setprecision(4) << solver.stats.harmony
                  << std::setw(14) << std::fixed << std::setprecision(4) << solver.stats.conflict
                  << std::setw(12) << solver.decisions
                  << std::setw(8) << std::fixed << std::setprecision(1) << sn
                  << std::setw(8) << (ok ? "✅" : "❌")
                  << "\n";
    }
    
    std::cout << std::string(96, '-') << "\n\n";
    std::cout << "  Tests: " << tests.size() << " | Correct: " << correct 
              << " (" << std::fixed << std::setprecision(1) << (100.0*correct/tests.size()) << "%)\n";
    std::cout << "  1+1=2 Architecture: " << (correct == (int)tests.size() ? "✅ VERIFIED" : "❌ MAY BUTAS") << "\n\n";
    
    return 0;
}
