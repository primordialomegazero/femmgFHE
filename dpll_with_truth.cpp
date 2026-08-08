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

// ============================================================
//  TRUTH VERIFIER - Brute force for small instances
// ============================================================

struct TruthVerifier {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    long long checked = 0;
    
    TruthVerifier(int n, const std::vector<std::vector<int>>& cls) 
        : n_vars(n), clauses(cls) {
        assignment.resize(n_vars + 1, 0);
    }
    
    bool check_assignment() {
        for (auto& cl : clauses) {
            bool sat = false;
            for (int lit : cl) {
                int v = abs(lit);
                if ((assignment[v] == 1 && lit > 0) || 
                    (assignment[v] == -1 && lit < 0)) {
                    sat = true;
                    break;
                }
            }
            if (!sat) return false;
        }
        return true;
    }
    
    bool brute(int var) {
        if (var > n_vars) {
            checked++;
            return check_assignment();
        }
        
        assignment[var] = 1;
        if (brute(var + 1)) return true;
        
        assignment[var] = -1;
        if (brute(var + 1)) return true;
        
        assignment[var] = 0;
        return false;
    }
    
    bool solve() {
        std::fill(assignment.begin(), assignment.end(), 0);
        checked = 0;
        return brute(1);
    }
};

// ============================================================
//  SIMPLE BUT CORRECT DPLL
// ============================================================

struct SimpleCorrectDPLL {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment, level, trail;
    long long decisions = 0;
    long long conflicts = 0;
    
    SimpleCorrectDPLL(int n, const std::vector<std::vector<int>>& cls) 
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
    
    bool propagate() {
        bool changed = true;
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
                    conflicts++;
                    return false;
                }
                
                if (unassigned == 1 && false_count == (int)cl.size() - 1) {
                    int v = abs(last_lit);
                    int val = (last_lit > 0) ? 1 : -1;
                    
                    if (assignment[v] != 0 && assignment[v] != val) {
                        conflicts++;
                        return false;
                    }
                    
                    assignment[v] = val;
                    level[v] = decisions;
                    trail.push_back(v);
                    changed = true;
                }
            }
        }
        return true;
    }
    
    bool solve_dpll() {
        if (!propagate()) return false;
        if (all_sat()) return true;
        
        // Pick first unassigned variable
        int v = -1;
        for (int i = 1; i <= n_vars; i++) {
            if (assignment[i] == 0) { v = i; break; }
        }
        if (v == -1) return false;
        
        decisions++;
        
        // Try true
        assignment[v] = 1;
        level[v] = decisions;
        trail.push_back(v);
        if (solve_dpll()) return true;
        
        // Backtrack
        while (!trail.empty() && level[trail.back()] >= decisions) {
            assignment[trail.back()] = 0;
            trail.pop_back();
        }
        
        // Try false
        assignment[v] = -1;
        level[v] = decisions;
        trail.push_back(v);
        if (solve_dpll()) return true;
        
        assignment[v] = 0;
        trail.pop_back();
        return false;
    }
    
    bool solve() {
        std::fill(assignment.begin(), assignment.end(), 0);
        std::fill(level.begin(), level.end(), -1);
        trail.clear();
        decisions = 0;
        conflicts = 0;
        return solve_dpll();
    }
};

// ============================================================
//  GENERATORS
// ============================================================

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

// ============================================================
//  MAIN - WITH TRUTH VERIFICATION
// ============================================================

int main() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  🔍 DPLL WITH TRUTH VERIFICATION - KNOW THE REAL ANSWER!            ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════╝\n\n";
    
    struct Test {
        std::string name;
        std::vector<std::vector<int>> clauses;
        int vars;
        bool truth; // Verified by brute force
    };
    
    std::vector<Test> tests;
    
    // Known UNSAT - verified by pigeonhole principle
    for (int n = 2; n <= 4; n++) {
        auto c = gen_pigeonhole(n);
        tests.push_back({"Pigeonhole " + std::to_string(n), c, (n+1)*n, false});
    }
    
    // Known results from graph coloring
    tests.push_back({"K3 2col", gen_graph_coloring(3,2), 6, false});
    tests.push_back({"K3 3col", gen_graph_coloring(3,3), 9, true});
    tests.push_back({"K4 2col", gen_graph_coloring(4,2), 8, false});
    tests.push_back({"K4 3col", gen_graph_coloring(4,3), 12, false});
    tests.push_back({"K5 2col", gen_graph_coloring(5,2), 10, false});
    tests.push_back({"K5 3col", gen_graph_coloring(5,3), 15, false});
    
    // Random instances - VERIFY with brute force
    std::cout << "Verifying random instances with brute force...\n";
    int seed = 42;
    std::vector<std::pair<int, int>> random_configs = {
        {10, 20}, {10, 25}, {10, 30},
        {15, 30}, {15, 35}, {15, 40}
    };
    
    for (auto [vars, cls] : random_configs) {
        auto c = gen_random_3sat(vars, cls, seed++);
        
        // Brute force to get truth
        TruthVerifier verifier(vars, c);
        auto start = std::chrono::high_resolution_clock::now();
        bool truth = verifier.solve();
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "  " << vars << "x" << cls << ": " << (truth ? "SAT" : "UNSAT") 
                  << " (checked " << verifier.checked << " assignments, " << ms.count() << "ms)\n";
        
        tests.push_back({"R" + std::to_string(vars) + "x" + std::to_string(cls), c, vars, truth});
    }
    
    std::cout << "\n";
    std::cout << std::left << std::setw(22) << "Test"
              << std::setw(8) << "Vars"
              << std::setw(8) << "Cls"
              << std::setw(12) << "DPLL Result"
              << std::setw(12) << "Truth"
              << std::setw(10) << "Decisions"
              << std::setw(8) << "Status"
              << "\n";
    std::cout << std::string(80, '-') << "\n";
    
    int correct = 0;
    for (auto& t : tests) {
        SimpleCorrectDPLL solver(t.vars, t.clauses);
        bool result = solver.solve();
        
        bool ok = (result == t.truth);
        if (ok) correct++;
        
        std::cout << std::left << std::setw(22) << t.name
                  << std::setw(8) << t.vars
                  << std::setw(8) << t.clauses.size()
                  << std::setw(12) << (result ? "SAT" : "UNSAT")
                  << std::setw(12) << (t.truth ? "SAT" : "UNSAT")
                  << std::setw(10) << solver.decisions
                  << std::setw(8) << (ok ? "✅" : "❌")
                  << "\n";
    }
    
    std::cout << std::string(80, '-') << "\n\n";
    double acc = (double)correct / tests.size() * 100;
    
    std::cout << "╔═══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  📊 RESULTS - WITH TRUTH VERIFICATION                              ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Tests:    " << std::setw(4) << tests.size() << "                                               ║\n";
    std::cout << "║  Correct:  " << std::setw(4) << correct << " (" << std::setw(6) << std::fixed << std::setprecision(2) << acc << "%)             ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════╝\n\n";
    
    if (acc >= 100.0) {
        std::cout << "🎉🎉🎉 100%! DPLL IS CORRECT! 🎉🎉🎉\n";
    } else {
        std::cout << "📈 " << std::fixed << std::setprecision(2) << acc << "% - DPLL has bugs!\n";
        std::cout << "   The simple DPLL should be 100% correct. Finding bugs...\n";
    }
    
    return 0;
}
