#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>

struct DPLLSolver {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assign, level, trail;
    int dl;
    long long nodes = 0, conflicts = 0;

    DPLLSolver(int n, const std::vector<std::vector<int>>& cls) : n_vars(n), clauses(cls), dl(0) {
        assign.resize(n_vars + 1, 0);
        level.resize(n_vars + 1, -1);
    }

    int lit_value(int lit) {
        int var = abs(lit);
        if (assign[var] == 0) return 0;
        return (assign[var] == 1) ? (lit > 0 ? 1 : -1) : (lit > 0 ? -1 : 1);
    }

    bool clause_satisfied(const std::vector<int>& cl) {
        for (int lit : cl) if (lit_value(lit) == 1) return true;
        return false;
    }

    int choose_var() {
        int best = 0, best_count = 0;
        for (int v = 1; v <= n_vars; v++) {
            if (assign[v] != 0) continue;
            int count = 0;
            for (auto& cl : clauses) {
                for (int lit : cl) if (abs(lit) == v) { count++; break; }
            }
            if (count > best_count) { best_count = count; best = v; }
        }
        return best;
    }

    bool bcp() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (auto& cl : clauses) {
                if (clause_satisfied(cl)) continue;
                int unassigned = 0, last_lit = 0;
                bool any_false = false;
                for (int lit : cl) {
                    int val = lit_value(lit);
                    if (val == 1) { any_false = false; break; }
                    if (val == 0) { unassigned++; last_lit = lit; }
                    if (val == -1) any_false = true;
                }
                if (clause_satisfied(cl)) continue;
                if (unassigned == 0 && any_false) { conflicts++; return false; }
                if (unassigned == 1 && !clause_satisfied(cl)) {
                    int var = abs(last_lit);
                    assign[var] = (last_lit > 0) ? 1 : -1;
                    level[var] = dl;
                    trail.push_back(var);
                    changed = true;
                }
            }
        }
        return true;
    }

    bool solve() {
        nodes++;
        if (!bcp()) return false;
        int var = choose_var();
        if (var == 0) return true;
        dl++;
        assign[var] = 1; level[var] = dl; trail.push_back(var);
        if (solve()) return true;
        assign[var] = -1; level[var] = dl;
        if (solve()) return true;
        assign[var] = 0; level[var] = -1; dl--;
        return false;
    }
};

// Generate UNSAT pigeonhole: n+1 pigeons, n holes
std::pair<int, std::vector<std::vector<int>>> pigeonhole(int n) {
    int pigeons = n + 1;
    int holes = n;
    int n_vars = pigeons * holes;
    std::vector<std::vector<int>> cls;
    
    // Each pigeon in at least one hole
    for (int p = 0; p < pigeons; p++) {
        std::vector<int> cl;
        for (int h = 0; h < holes; h++) {
            cl.push_back(p * holes + h + 1);
        }
        cls.push_back(cl);
    }
    
    // No two pigeons in same hole
    for (int h = 0; h < holes; h++) {
        for (int p1 = 0; p1 < pigeons; p1++) {
            for (int p2 = p1 + 1; p2 < pigeons; p2++) {
                cls.push_back({-(p1 * holes + h + 1), -(p2 * holes + h + 1)});
            }
        }
    }
    
    return {n_vars, cls};
}

int main() {
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  SCALED UNSAT: Pigeonhole Problems\n";
    std::cout << "============================================================\n\n";

    std::cout << std::left << std::setw(12) << "Problem"
              << std::setw(10) << "Vars"
              << std::setw(10) << "Clauses"
              << std::setw(12) << "Expected"
              << std::setw(8) << "Result"
              << std::setw(10) << "Nodes"
              << std::setw(10) << "Time"
              << "Status\n";
    std::cout << std::string(72, '-') << "\n";

    int sizes[] = {2, 3, 4, 5, 6};
    int passed = 0;
    
    for (int n : sizes) {
        auto [n_vars, cls] = pigeonhole(n);
        
        auto start = std::chrono::steady_clock::now();
        DPLLSolver solver(n_vars, cls);
        bool result = solver.solve();
        auto end = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        bool ok = (result == false);
        if (ok) passed++;
        
        std::cout << std::left << std::setw(12) << (std::to_string(n+1) + "p/" + std::to_string(n) + "h")
                  << std::setw(10) << n_vars
                  << std::setw(10) << cls.size()
                  << std::setw(12) << "UNSAT"
                  << std::setw(8) << (result ? "SAT" : "UNSAT")
                  << std::setw(10) << solver.nodes
                  << std::setw(10) << (std::to_string(ms) + "ms")
                  << (ok ? "OK" : "FAIL") << "\n";
    }

    std::cout << "\n  Pigeonhole: " << passed << "/" << (sizeof(sizes)/sizeof(sizes[0])) << " passed\n";
    std::cout << "============================================================\n\n";

    return (passed == (int)(sizeof(sizes)/sizeof(sizes[0]))) ? 0 : 1;
}
