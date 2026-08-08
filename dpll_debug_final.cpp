#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <random>
#include <set>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

struct DebugDPLL {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment, level, trail;
    std::vector<double> phi_score, psi_score;
    bool debug = true;
    
    struct Stats {
        long long decisions = 0;
        long long propagations = 0;
        long long conflicts = 0;
        long long backtracks = 0;
        int depth = 0;
        double phi_ratio = 0;
        double psi_ratio = 0;
    } stats;
    
    DebugDPLL(int n, const std::vector<std::vector<int>>& cls) 
        : n_vars(n), clauses(cls) {
        assignment.resize(n_vars + 1, 0);
        level.resize(n_vars + 1, -1);
        phi_score.resize(n_vars + 1, 1.0);
        psi_score.resize(n_vars + 1, 1.0);
        trail.reserve(n_vars + 1);
    }
    
    int lit_val(int lit) {
        int v = abs(lit);
        if (assignment[v] == 0) return 0;
        return (assignment[v] == 1) ? (lit > 0 ? 1 : -1) : (lit > 0 ? -1 : 1);
    }
    
    bool clause_sat(const std::vector<int>& cl) {
        for (int lit : cl) {
            if (lit_val(lit) == 1) return true;
        }
        return false;
    }
    
    bool all_sat() {
        for (auto& cl : clauses) {
            if (!clause_sat(cl)) return false;
        }
        return true;
    }
    
    void print_assignment() {
        std::cout << "  Assignment: ";
        int count = 0;
        for (int v = 1; v <= n_vars && count < 15; v++) {
            if (assignment[v] != 0) {
                std::cout << "x" << v << "=" << (assignment[v] == 1 ? "T" : "F") << " ";
                count++;
            }
        }
        if (count < n_vars) std::cout << "...";
        std::cout << "\n";
    }
    
    void print_clause_status() {
        int sat = 0, unsat = 0, unit = 0;
        for (auto& cl : clauses) {
            if (clause_sat(cl)) sat++;
            else {
                unsat++;
                int unassigned = 0;
                for (int lit : cl) {
                    if (lit_val(lit) == 0) unassigned++;
                }
                if (unassigned == 1) unit++;
            }
        }
        std::cout << "  Clause status: SAT=" << sat << ", UNSAT=" << unsat << ", UNIT=" << unit << "\n";
    }
    
    bool propagate() {
        bool changed = true;
        int prop_count = 0;
        while (changed) {
            changed = false;
            for (auto& cl : clauses) {
                if (clause_sat(cl)) continue;
                
                int unassigned = 0;
                int last_lit = 0;
                int false_count = 0;
                
                for (int lit : cl) {
                    int val = lit_val(lit);
                    if (val == 0) { 
                        unassigned++; 
                        last_lit = lit; 
                    } else if (val == -1) {
                        false_count++;
                    }
                }
                
                if (unassigned == 0 && false_count == (int)cl.size()) {
                    stats.conflicts++;
                    if (debug) {
                        std::cout << "  ⚠️  CONFLICT!\n";
                        print_assignment();
                    }
                    return false;
                }
                
                if (unassigned == 1 && false_count == (int)cl.size() - 1) {
                    int v = abs(last_lit);
                    int val = (last_lit > 0) ? 1 : -1;
                    
                    if (assignment[v] != 0 && assignment[v] != val) {
                        stats.conflicts++;
                        return false;
                    }
                    
                    assignment[v] = val;
                    level[v] = stats.decisions;
                    trail.push_back(v);
                    stats.propagations++;
                    prop_count++;
                    changed = true;
                    
                    if (debug) {
                        std::cout << "  📌 Unit: x" << v << "=" << (val == 1 ? "T" : "F") << "\n";
                    }
                    
                    if (val == 1) phi_score[v] += 1.0;
                    else psi_score[v] += 1.0;
                }
            }
        }
        if (debug && prop_count > 0) {
            std::cout << "  Propagations: " << prop_count << "\n";
            print_assignment();
        }
        return true;
    }
    
    int select_var() {
        int best = -1;
        double best_score = -1e9;
        
        if (debug) std::cout << "  Selecting variable:\n";
        for (int v = 1; v <= n_vars; v++) {
            if (assignment[v] != 0) continue;
            
            double phi = phi_score[v];
            double psi = psi_score[v];
            double score = phi * PHI + psi * PSI;
            
            int occ = 0;
            for (auto& cl : clauses) {
                if (clause_sat(cl)) continue;
                for (int lit : cl) {
                    if (abs(lit) == v) occ++;
                }
            }
            score += occ * 0.5;
            
            if (debug && v <= 8) {
                std::cout << "    x" << v << ": φ=" << std::fixed << std::setprecision(2) << phi 
                          << " ψ=" << psi << " occ=" << occ << " score=" << score << "\n";
            }
            
            if (score > best_score) {
                best_score = score;
                best = v;
            }
        }
        if (debug) std::cout << "  Selected: x" << best << "\n";
        return best;
    }
    
    bool solve_dpll(int depth = 0) {
        stats.depth = std::max(stats.depth, depth);
        
        if (debug && depth < 10) {
            std::cout << "\n" << std::string(40, '-') << "\n";
            std::cout << "  Depth: " << depth << ", Decisions: " << stats.decisions << "\n";
            print_assignment();
            print_clause_status();
        }
        
        if (!propagate()) {
            if (debug) std::cout << "  ❌ Propagation failed\n";
            return false;
        }
        
        if (all_sat()) {
            if (debug) std::cout << "  ✅ ALL SAT!\n";
            return true;
        }
        
        int v = select_var();
        if (v == -1) {
            if (debug) std::cout << "  ❌ No variable\n";
            return false;
        }
        
        stats.decisions++;
        
        // Try positive
        if (debug) std::cout << "  🔄 Try x" << v << "=T\n";
        assignment[v] = 1;
        level[v] = stats.decisions;
        trail.push_back(v);
        
        if (solve_dpll(depth + 1)) return true;
        
        // Backtrack
        if (debug) std::cout << "  ↩️  Backtrack x" << v << "=T\n";
        while (!trail.empty() && level[trail.back()] >= stats.decisions) {
            assignment[trail.back()] = 0;
            trail.pop_back();
        }
        
        // Try negative
        if (debug) std::cout << "  🔄 Try x" << v << "=F\n";
        assignment[v] = -1;
        level[v] = stats.decisions;
        trail.push_back(v);
        
        if (solve_dpll(depth + 1)) return true;
        
        // Backtrack
        if (debug) std::cout << "  ↩️  Backtrack x" << v << "=F\n";
        while (!trail.empty() && level[trail.back()] >= stats.decisions) {
            assignment[trail.back()] = 0;
            trail.pop_back();
        }
        stats.backtracks++;
        return false;
    }
    
    bool solve() {
        std::fill(assignment.begin(), assignment.end(), 0);
        std::fill(level.begin(), level.end(), -1);
        trail.clear();
        stats = Stats();
        for (int i = 1; i <= n_vars; i++) {
            phi_score[i] = 1.0;
            psi_score[i] = 1.0;
        }
        return solve_dpll();
    }
};

// Generators
std::vector<std::vector<int>> gen_pigeonhole(int n) {
    std::vector<std::vector<int>> cls;
    for (int p = 0; p < n+1; p++) {
        std::vector<int> cl;
        for (int h = 0; h < n; h++) cl.push_back(p * n + h + 1);
        cls.push_back(cl);
    }
    for (int h = 0; h < n; h++) {
        for (int p1 = 0; p1 < n+1; p1++) {
            for (int p2 = p1+1; p2 < n+1; p2++) {
                cls.push_back({-(p1 * n + h + 1), -(p2 * n + h + 1)});
            }
        }
    }
    return cls;
}

std::vector<std::vector<int>> gen_graph_coloring(int v, int c) {
    std::vector<std::vector<int>> cls;
    for (int i = 0; i < v; i++) {
        std::vector<int> cl;
        for (int j = 0; j < c; j++) cl.push_back(i * c + j + 1);
        cls.push_back(cl);
    }
    for (int i = 0; i < v; i++) {
        for (int j1 = 0; j1 < c; j1++) {
            for (int j2 = j1+1; j2 < c; j2++) {
                cls.push_back({-(i * c + j1 + 1), -(i * c + j2 + 1)});
            }
        }
    }
    for (int i1 = 0; i1 < v; i1++) {
        for (int i2 = i1+1; i2 < v; i2++) {
            for (int j = 0; j < c; j++) {
                cls.push_back({-(i1 * c + j + 1), -(i2 * c + j + 1)});
            }
        }
    }
    return cls;
}

std::vector<std::vector<int>> gen_random_3sat(int v, int c, int seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> vd(1, v);
    std::uniform_int_distribution<int> sd(0, 1);
    std::vector<std::vector<int>> res;
    std::set<std::string> seen;
    for (int i = 0; i < c * 2 && (int)res.size() < c; i++) {
        std::vector<int> cl;
        std::string key;
        for (int j = 0; j < 3; j++) {
            int x = vd(rng);
            if (sd(rng)) x = -x;
            cl.push_back(x);
            key += std::to_string(x) + ",";
        }
        if (seen.find(key) == seen.end()) {
            res.push_back(cl);
            seen.insert(key);
        }
    }
    return res;
}

int main() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  🔍 DETAILED DEBUG - R3SAT 30x90                                   ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════╝\n\n";
    
    auto clauses = gen_random_3sat(30, 90, 42);
    DebugDPLL solver(30, clauses);
    
    std::cout << "Instance: 30 vars, " << clauses.size() << " clauses\n";
    std::cout << "Density: " << std::fixed << std::setprecision(2) << (double)clauses.size() / 30 << "\n\n";
    std::cout << "=== STARTING DETAILED SOLVE ===\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    bool result = solver.solve();
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\n=== RESULT ===\n";
    std::cout << "Result: " << (result ? "SAT ✅" : "UNSAT ❌") << "\n";
    std::cout << "Decisions: " << solver.stats.decisions << "\n";
    std::cout << "Propagations: " << solver.stats.propagations << "\n";
    std::cout << "Conflicts: " << solver.stats.conflicts << "\n";
    std::cout << "Backtracks: " << solver.stats.backtracks << "\n";
    std::cout << "Max depth: " << solver.stats.depth << "\n";
    std::cout << "Time: " << ms.count() << "ms\n";
    
    return 0;
}
