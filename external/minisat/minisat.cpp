// ═══════════════════════════════════════════════════════════════
// MINISAT — Fixed Unit Clause Handling
// ═══════════════════════════════════════════════════════════════

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <chrono>

struct MiniSAT {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assign, level, reason;
    std::vector<std::vector<int>> watchers;
    std::vector<int> watch1, watch2;
    std::vector<double> activity;
    int dl;
    
    long long nodes = 0, conflicts = 0, decisions = 0;
    std::vector<int> model;
    bool sat = false;
    
    MiniSAT(int n, const std::vector<std::vector<int>>& cls) : n_vars(n), clauses(cls), dl(0) {
        assign.resize(n_vars + 1, 0);
        level.resize(n_vars + 1, -1);
        reason.resize(n_vars + 1, -1);
        activity.resize(n_vars + 1, 0.0);
        watchers.resize(2 * (n_vars + 1) + 1);
        watch1.resize(clauses.size(), -1);
        watch2.resize(clauses.size(), -1);
        init_watchers();
    }
    
    int lit_idx(int lit) {
        int v = std::abs(lit);
        return lit > 0 ? v : n_vars + v;
    }
    
    void init_watchers() {
        for (size_t i = 0; i < clauses.size(); i++) {
            if (clauses[i].size() == 1) {
                // Unit clause — watch the single literal
                watch1[i] = 0;
                watch2[i] = -1;
                watchers[lit_idx(clauses[i][0])].push_back(i);
            } else if (clauses[i].size() >= 2) {
                watch1[i] = 0; 
                watch2[i] = 1;
                watchers[lit_idx(clauses[i][0])].push_back(i);
                watchers[lit_idx(clauses[i][1])].push_back(i);
            }
        }
    }
    
    int val(int lit) {
        int v = std::abs(lit);
        if (assign[v] == 0) return 0;
        return (lit > 0) == (assign[v] == 1) ? 1 : -1;
    }
    
    bool bcp() {
        // Queue ALL newly assigned variables
        std::vector<int> prop_queue;
        for (int v = 1; v <= n_vars; v++)
            if (assign[v] != 0 && level[v] == dl) 
                prop_queue.push_back(v);
        
        while (!prop_queue.empty()) {
            int p = prop_queue.back(); 
            prop_queue.pop_back();
            
            // The literal that just became FALSE
            int false_lit = (assign[p] == 1) ? -p : p;
            int fl_idx = lit_idx(false_lit);
            
            std::vector<int> to_check;
            std::swap(to_check, watchers[fl_idx]);
            
            for (int ci : to_check) {
                // Determine which watch is the false literal
                int false_pos = -1, other_pos = -1;
                
                if (watch1[ci] >= 0 && watch1[ci] < (int)clauses[ci].size() && 
                    clauses[ci][watch1[ci]] == false_lit) {
                    false_pos = watch1[ci];
                    other_pos = watch2[ci];
                } else if (watch2[ci] >= 0 && watch2[ci] < (int)clauses[ci].size() && 
                           clauses[ci][watch2[ci]] == false_lit) {
                    false_pos = watch2[ci];
                    other_pos = watch1[ci];
                } else {
                    // This clause no longer watches this literal, re-add
                    watchers[fl_idx].push_back(ci);
                    continue;
                }
                
                // If other watch is satisfied, clause is fine
                if (other_pos >= 0 && other_pos < (int)clauses[ci].size()) {
                    if (val(clauses[ci][other_pos]) == 1) {
                        watchers[fl_idx].push_back(ci);
                        continue;
                    }
                }
                
                // Try to find a new watcher
                bool found = false;
                for (int i = 0; i < (int)clauses[ci].size(); i++) {
                    if (i == false_pos || i == other_pos) continue;
                    if (val(clauses[ci][i]) != -1) {
                        if (false_pos == watch1[ci]) watch1[ci] = i;
                        else watch2[ci] = i;
                        watchers[lit_idx(clauses[ci][i])].push_back(ci);
                        found = true;
                        break;
                    }
                }
                if (found) continue;
                
                // Cannot find new watcher — check other watch
                if (other_pos == -1) {
                    // Unit clause with single FALSE literal = CONFLICT
                    conflicts++;
                    return false;
                }
                
                int ov = val(clauses[ci][other_pos]);
                if (ov == -1) {
                    // Both watches are FALSE = CONFLICT
                    conflicts++;
                    return false;
                }
                
                if (ov == 0) {
                    // UNIT PROPAGATION
                    int lit = clauses[ci][other_pos];
                    int v = std::abs(lit);
                    assign[v] = (lit > 0) ? 1 : -1;
                    level[v] = dl;
                    reason[v] = ci;
                    prop_queue.push_back(v);
                    watchers[fl_idx].push_back(ci);
                }
            }
        }
        return true;
    }
    
    int select_var() {
        std::vector<double> scores(n_vars + 1, 0.0);
        for (size_t ci = 0; ci < clauses.size(); ci++) {
            bool sat = false; 
            int undef = 0;
            for (int lit : clauses[ci]) { 
                if (val(lit) == 1) { sat = true; break; } 
                if (val(lit) == 0) undef++; 
            }
            if (sat) continue;
            double w = 1.0 / (undef + 1);
            for (int lit : clauses[ci]) 
                if (val(lit) == 0) scores[std::abs(lit)] += w;
        }
        int best = -1; 
        double best_s = -1;
        for (int v = 1; v <= n_vars; v++) {
            if (assign[v] == 0) {
                double s = scores[v] + activity[v] * 0.01;
                if (s > best_s) { best_s = s; best = v; }
            }
        }
        return best;
    }
    
    void backtrack(int target) {
        for (int v = 1; v <= n_vars; v++) {
            if (level[v] > target) { 
                assign[v] = 0; 
                level[v] = -1; 
                reason[v] = -1; 
            }
        }
        dl = target;
    }
    
    bool solve(int max_nodes = 50000000) {
        // Initial BCP at level 0 to propagate unit clauses
        if (!bcp()) return false;
        
        while (true) {
            nodes++; 
            if (nodes > max_nodes) return false;
            
            bool all_done = true;
            for (int v = 1; v <= n_vars; v++) 
                if (assign[v] == 0) { all_done = false; break; }
            
            if (all_done) { 
                model = assign; 
                sat = true; 
                return true; 
            }
            
            int var = select_var(); 
            if (var == -1) return false;
            
            decisions++; 
            dl++;
            assign[var] = 1; 
            level[var] = dl; 
            reason[var] = -1;
            activity[var] += 1.0;
            
            if (!bcp()) {
                // Conflict — try opposite polarity
                for (int v = 1; v <= n_vars; v++) {
                    if (level[v] == dl) { assign[v] = 0; level[v] = -1; reason[v] = -1; }
                }
                assign[var] = -1; 
                level[var] = dl;
                
                if (!bcp()) {
                    // Both polarities conflict — backtrack
                    conflicts++;
                    if (dl == 0) return false;
                    for (int v = 1; v <= n_vars; v++) 
                        activity[v] *= 0.95;
                    backtrack(dl - 1);
                }
            }
        }
    }
};

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: minisat <dimacs.cnf>\n"; return 1; }
    
    std::ifstream f(argv[1]);
    std::string line;
    int n_vars = 0, n_clauses = 0;
    std::vector<std::vector<int>> cls;
    
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') { 
            sscanf(line.c_str(), "p cnf %d %d", &n_vars, &n_clauses); 
            continue; 
        }
        std::stringstream ss(line);
        std::vector<int> cl; 
        int lit;
        while (ss >> lit && lit != 0) cl.push_back(lit);
        if (!cl.empty()) cls.push_back(cl);
    }
    f.close();
    
    std::cout << "MiniSAT: " << n_vars << " vars, " << cls.size() << " clauses\n";
    
    MiniSAT solver(n_vars, cls);
    auto t1 = std::chrono::steady_clock::now();
    bool result = solver.solve();
    auto t2 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double>(t2 - t1).count() * 1000;
    
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n";
    std::cout << "Nodes: " << solver.nodes << " | Conflicts: " << solver.conflicts << "\n";
    std::cout << "Time: " << std::fixed << std::setprecision(2) << ms << "ms\n";
    
    if (result) {
        std::cout << "Model (first 20 vars): ";
        for (int v = 1; v <= std::min(20, n_vars); v++) 
            std::cout << (solver.model[v] == 1 ? "1" : "0");
        std::cout << "\n";
    }
    
    return result ? 10 : 20;
}
