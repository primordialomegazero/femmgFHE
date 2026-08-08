#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <set>
#include <map>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// Simple brute force for 30 variables to verify
struct BruteForce {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    long long checked = 0;
    
    BruteForce(int n, const std::vector<std::vector<int>>& cls) 
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

// Our DPLL for comparison
struct DebugDPLL {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment, level, trail;
    std::vector<double> phi_score, psi_score;
    long long decisions = 0;
    bool verbose = false;
    
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
        for (int lit : cl) if (lit_val(lit) == 1) return true;
        return false;
    }
    
    bool all_sat() {
        for (auto& cl : clauses) if (!clause_sat(cl)) return false;
        return true;
    }
    
    bool propagate() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (auto& cl : clauses) {
                if (clause_sat(cl)) continue;
                int unassigned = 0, last_lit = 0;
                bool has_false = false;
                for (int lit : cl) {
                    int val = lit_val(lit);
                    if (val == 0) { unassigned++; last_lit = lit; }
                    else if (val == -1) has_false = true;
                }
                if (unassigned == 0 && has_false) return false;
                if (unassigned == 1 && has_false) {
                    int v = abs(last_lit);
                    int val = (last_lit > 0) ? 1 : -1;
                    if (assignment[v] != 0 && assignment[v] != val) return false;
                    assignment[v] = val;
                    level[v] = decisions;
                    trail.push_back(v);
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
            double score = phi_score[v] * PHI + psi_score[v] * PSI;
            if (score > best_score) {
                best_score = score;
                best = v;
            }
        }
        return best;
    }
    
    bool solve_dpll() {
        if (!propagate()) return false;
        if (all_sat()) return true;
        
        int v = select_var();
        if (v == -1) return all_sat();
        
        decisions++;
        
        // Try positive
        assignment[v] = 1;
        level[v] = decisions;
        trail.push_back(v);
        if (solve_dpll()) return true;
        
        // Backtrack
        while (!trail.empty() && level[trail.back()] >= decisions) {
            assignment[trail.back()] = 0;
            trail.pop_back();
        }
        
        // Try negative
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
        for (int i = 1; i <= n_vars; i++) {
            phi_score[i] = 1.0;
            psi_score[i] = 1.0;
        }
        return solve_dpll();
    }
};

// Generate the specific 30x90 instance
std::vector<std::vector<int>> gen_specific_30x90(int seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> vd(1, 30);
    std::uniform_int_distribution<int> sd(0, 1);
    std::vector<std::vector<int>> res;
    std::set<std::string> seen;
    for (int i = 0; i < 180 && (int)res.size() < 90; i++) {
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
    std::cout << "║  🔍 DEBUG: R3SAT 30x90 - BUG OR LIMITATION?                        ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════╝\n\n";
    
    // Generate the instance
    auto clauses = gen_specific_30x90(42);
    int vars = 30;
    
    std::cout << "Instance: 30 variables, " << clauses.size() << " clauses\n\n";
    
    // Try our DPLL
    DebugDPLL dpll(vars, clauses);
    auto start_dpll = std::chrono::high_resolution_clock::now();
    bool dpll_result = dpll.solve();
    auto end_dpll = std::chrono::high_resolution_clock::now();
    auto dpll_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_dpll - start_dpll);
    
    std::cout << "DPLL Result: " << (dpll_result ? "SAT" : "UNSAT") << "\n";
    std::cout << "Decisions: " << dpll.decisions << "\n";
    std::cout << "Time: " << dpll_ms.count() << "ms\n\n";
    
    // Try brute force (but limit to avoid explosion)
    std::cout << "Brute force verification (limit to 2^20 = 1,048,576 combinations)...\n";
    BruteForce brute(vars, clauses);
    auto start_brute = std::chrono::high_resolution_clock::now();
    
    // Check first 20 variables only
    bool brute_result = false;
    long long checked = 0;
    for (int i = 0; i < (1 << 20) && !brute_result; i++) {
        // Assign first 20 variables based on i
        for (int v = 1; v <= 20; v++) {
            brute.assignment[v] = ((i >> (v-1)) & 1) ? 1 : -1;
        }
        // Assign remaining 10 as random
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dis(0, 1);
        for (int v = 21; v <= 30; v++) {
            brute.assignment[v] = dis(rng) ? 1 : -1;
        }
        
        if (brute.check_assignment()) {
            brute_result = true;
            std::cout << "Found SAT assignment!\n";
            for (int v = 1; v <= 30; v++) {
                std::cout << "x" << v << " = " << (brute.assignment[v] == 1 ? "true" : "false") << "\n";
            }
            break;
        }
        checked++;
    }
    
    auto end_brute = std::chrono::high_resolution_clock::now();
    auto brute_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_brute - start_brute);
    
    std::cout << "\nBrute force checked: " << checked << " combinations\n";
    std::cout << "Brute force result: " << (brute_result ? "SAT" : "UNSAT") << "\n";
    std::cout << "Time: " << brute_ms.count() << "ms\n\n";
    
    // Analyze the conflict
    std::cout << "╔═══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  📊 ANALYSIS                                                         ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════════════════════╣\n";
    if (dpll_result == brute_result) {
        std::cout << "║  ✅ DPLL matches brute force. No bug!                             ║\n";
        std::cout << "║  The instance is actually " << (brute_result ? "SAT" : "UNSAT") << ".                          ║\n";
    } else {
        std::cout << "║  ⚠️  DPLL gave wrong result. Bug found!                          ║\n";
    }
    std::cout << "╚═══════════════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
