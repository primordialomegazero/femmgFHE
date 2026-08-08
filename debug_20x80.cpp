#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <set>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// ============================================================
//  DPLL DEBUGGER - Focus on 20x80
// ============================================================

struct DebugDPLL {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment, level, trail;
    bool debug = true;
    
    struct Stats {
        long long decisions = 0;
        long long propagations = 0;
        long long conflicts = 0;
    } stats;
    
    DebugDPLL(int n, const std::vector<std::vector<int>>& cls) 
        : n_vars(n), clauses(cls) {
        assignment.resize(n_vars + 1, 0);
        level.resize(n_vars + 1, -1);
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
        for (int v = 1; v <= std::min(n_vars, 15); v++) {
            std::cout << "x" << v << "=" << (assignment[v] == 1 ? "T" : assignment[v] == -1 ? "F" : "?") << " ";
        }
        if (n_vars > 15) std::cout << "...";
        std::cout << "\n";
    }
    
    void print_unsat_clauses() {
        int count = 0;
        std::cout << "  Unsat clauses (first 5):\n";
        for (size_t i = 0; i < clauses.size() && count < 5; i++) {
            if (!clause_sat(clauses[i])) {
                std::cout << "    Clause " << i << ": ";
                for (int lit : clauses[i]) {
                    std::cout << lit << " ";
                }
                std::cout << "\n";
                count++;
            }
        }
        if (count == 0) std::cout << "    None! All satisfied.\n";
    }
    
    bool propagate() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (size_t idx = 0; idx < clauses.size(); idx++) {
                auto& cl = clauses[idx];
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
                        std::cout << "  ⚠️ CONFLICT at clause " << idx << ": ";
                        for (int lit : cl) std::cout << lit << " ";
                        std::cout << "\n";
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
                    changed = true;
                    
                    if (debug) {
                        std::cout << "  📌 Unit propagation: x" << v << " = " << (val == 1 ? "TRUE" : "FALSE") << "\n";
                    }
                }
            }
        }
        return true;
    }
    
    int select_var() {
        // Simple: first unassigned
        for (int v = 1; v <= n_vars; v++) {
            if (assignment[v] == 0) return v;
        }
        return -1;
    }
    
    bool dpll(int depth = 0) {
        if (debug && depth < 10) {
            std::cout << "\n" << std::string(40, '-') << "\n";
            std::cout << "  Depth: " << depth << ", Decisions: " << stats.decisions << "\n";
            print_assignment();
            print_unsat_clauses();
        }
        
        if (!propagate()) {
            if (debug) std::cout << "  ❌ Propagation failed\n";
            return false;
        }
        
        if (all_sat()) {
            if (debug) std::cout << "  ✅ ALL SATISFIED!\n";
            return true;
        }
        
        int v = select_var();
        if (v == -1) {
            if (debug) std::cout << "  ❌ No variable to assign\n";
            return false;
        }
        
        stats.decisions++;
        
        if (debug) std::cout << "  🔄 Trying x" << v << " = TRUE\n";
        assignment[v] = 1;
        level[v] = stats.decisions;
        trail.push_back(v);
        if (dpll(depth + 1)) return true;
        
        // Backtrack
        if (debug) std::cout << "  ↩️ Backtracking from x" << v << " = TRUE\n";
        while (!trail.empty() && level[trail.back()] >= stats.decisions) {
            assignment[trail.back()] = 0;
            trail.pop_back();
        }
        
        if (debug) std::cout << "  🔄 Trying x" << v << " = FALSE\n";
        assignment[v] = -1;
        level[v] = stats.decisions;
        trail.push_back(v);
        if (dpll(depth + 1)) return true;
        
        // Backtrack
        if (debug) std::cout << "  ↩️ Backtracking from x" << v << " = FALSE\n";
        while (!trail.empty() && level[trail.back()] >= stats.decisions) {
            assignment[trail.back()] = 0;
            trail.pop_back();
        }
        return false;
    }
    
    bool solve() {
        std::fill(assignment.begin(), assignment.end(), 0);
        std::fill(level.begin(), level.end(), -1);
        trail.clear();
        stats = Stats();
        return dpll();
    }
};

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
    std::cout << "║  🔍 DEBUGGING R3SAT 20x80                                          ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════╝\n\n";
    
    // Generate 20x80
    auto clauses = gen_random_3sat(20, 80, 42);
    DebugDPLL solver(20, clauses);
    
    std::cout << "Instance: 20 variables, " << clauses.size() << " clauses\n";
    std::cout << "Density: " << std::fixed << std::setprecision(2) << (double)clauses.size() / 20 << "\n\n";
    std::cout << "=== STARTING DETAILED SOLVE ===\n";
    
    bool result = solver.solve();
    
    std::cout << "\n=== RESULT ===\n";
    std::cout << "Result: " << (result ? "SAT ✅" : "UNSAT ❌") << "\n";
    std::cout << "Decisions: " << solver.stats.decisions << "\n";
    std::cout << "Propagations: " << solver.stats.propagations << "\n";
    std::cout << "Conflicts: " << solver.stats.conflicts << "\n";
    
    return 0;
}
