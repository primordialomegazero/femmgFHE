#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>

const double PHI = 1.6180339887498948482;

struct PhiWatcher {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assign;
    std::vector<int> level;
    std::vector<int> trail;
    int dl;
    long long nodes = 0, bcp_units = 0, conflicts = 0;
    long long decisions_count = 0;

    PhiWatcher(int n, const std::vector<std::vector<int>>& cls) : n_vars(n), clauses(cls), dl(0) {
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
        for (int v = 1; v <= n_vars; v++) {
            if (assign[v] == 0) {
                for (auto& cl : clauses) {
                    for (int lit : cl) {
                        if (abs(lit) == v) return v;
                    }
                }
            }
        }
        return 0;
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
                
                if (unassigned == 0 && any_false) {
                    conflicts++;
                    return false;
                }
                
                if (unassigned == 1 && !clause_satisfied(cl)) {
                    int var = abs(last_lit);
                    assign[var] = (last_lit > 0) ? 1 : -1;
                    level[var] = dl;
                    trail.push_back(var);
                    bcp_units++;
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
        decisions_count++;
        assign[var] = 1;
        level[var] = dl;
        trail.push_back(var);
        if (solve()) return true;
        assign[var] = -1;
        level[var] = dl;
        if (solve()) return true;
        assign[var] = 0;
        level[var] = -1;
        dl--;
        return false;
    }
};

int main() {
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  PHI-DPLL: SAT/UNSAT Test\n";
    std::cout << "============================================================\n\n";

    struct TestCase {
        std::string name;
        int n_vars;
        std::vector<std::vector<int>> clauses;
        bool expected;
    };

    std::vector<TestCase> tests = {
        {"SAT: x1 AND x2", 2, {{1}, {2}}, true},
        {"UNSAT: x1 AND NOT x1", 1, {{1}, {-1}}, false},
        {"SAT: (x1 OR x2) AND (NOT x1 OR x2)", 2, {{1, 2}, {-1, 2}}, true},
        {"UNSAT: (x1) AND (NOT x1 OR x2) AND (NOT x2)", 2, {{1}, {-1, 2}, {-2}}, false},
        {"UNSAT: Pigeonhole 3 pigeons, 2 holes", 6,
         {{1,2},{3,4},{5,6},{-1,-3},{-1,-5},{-3,-5},{-2,-4},{-2,-6},{-4,-6}}, false},
    };

    int passed = 0;
    for (auto& t : tests) {
        auto start = std::chrono::steady_clock::now();
        PhiWatcher solver(t.n_vars, t.clauses);
        bool result = solver.solve();
        auto end = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        bool ok = (result == t.expected);
        if (ok) passed++;
        
        std::cout << "  " << std::left << std::setw(48) << t.name
                  << " Exp:" << (t.expected?"SAT":"UNSAT")
                  << " Got:" << (result?"SAT":"UNSAT")
                  << " Nodes:" << solver.nodes
                  << " Time:" << ms << "ms"
                  << " " << (ok?"OK":"FAIL") << "\n";
    }

    std::cout << "\n  Result: " << passed << "/" << tests.size() << " passed\n";
    std::cout << "============================================================\n\n";

    return (passed == (int)tests.size()) ? 0 : 1;
}
