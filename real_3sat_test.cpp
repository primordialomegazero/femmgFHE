#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <random>
#include <set>
using namespace std;

struct DPLL {
    int nodes = 0;
    vector<vector<int>> clauses;
    vector<int> assign;
    
    bool solve(int n, const vector<vector<int>>& cls) {
        clauses = cls;
        assign.assign(n, -1);
        nodes = 0;
        return dpll();
    }
    
    bool dpll() {
        nodes++;
        
        // Unit propagation
        bool changed;
        do {
            changed = false;
            for (auto& cl : clauses) {
                int unassigned = 0, last = 0;
                bool sat = false;
                for (int lit : cl) {
                    int v = abs(lit)-1;
                    if (assign[v] == -1) { unassigned++; last = lit; }
                    else {
                        bool val = (assign[v] == 1) == (lit > 0);
                        if (val) { sat = true; break; }
                    }
                }
                if (!sat && unassigned == 0) return false;
                if (!sat && unassigned == 1) {
                    assign[abs(last)-1] = (last > 0) ? 1 : 0;
                    changed = true;
                }
            }
        } while (changed);
        
        // Check if satisfied
        bool all_sat = true;
        for (auto& cl : clauses) {
            bool sat = false;
            for (int lit : cl) {
                int v = abs(lit)-1;
                if (assign[v] == -1) { all_sat = false; continue; }
                bool val = (assign[v] == 1) == (lit > 0);
                if (val) { sat = true; break; }
            }
            if (!sat) return false;
        }
        if (all_sat) return true;
        
        // Choose variable (simple heuristic)
        int var = -1;
        for (int i = 0; i < (int)assign.size(); i++) {
            if (assign[i] == -1) { var = i; break; }
        }
        if (var == -1) return true;
        
        // Try true
        assign[var] = 1;
        if (dpll()) return true;
        
        // Try false
        assign[var] = 0;
        if (dpll()) return true;
        
        assign[var] = -1;
        return false;
    }
};

vector<vector<int>> gen_random_3sat(int vars, int clauses) {
    vector<vector<int>> cls;
    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> var_dist(1, vars);
    uniform_int_distribution<int> sign_dist(0, 1);
    
    for (int c = 0; c < clauses; c++) {
        set<int> used;
        vector<int> cl;
        for (int l = 0; l < 3; l++) {
            int v;
            do { v = var_dist(rng); } while (used.count(v));
            used.insert(v);
            cl.push_back(sign_dist(rng) ? v : -v);
        }
        cls.push_back(cl);
    }
    return cls;
}

int main() {
    cout << "\n  ╔════════════════════════════════════════════════╗\n";
    cout << "  ║  REAL 3-SAT TEST - Exponential Growth        ║\n";
    cout << "  ║  Ito ang totoong NP-complete problem         ║\n";
    cout << "  ╚════════════════════════════════════════════════╝\n\n";
    
    cout << "  Vars\tClauses\tNodes\tTime(ms)\tResult\n";
    cout << "  " << string(55, '-') << "\n";
    
    for (int vars = 10; vars <= 25; vars += 5) {
        for (int cls = vars * 3; cls <= vars * 4; cls += vars) {
            auto cnf = gen_random_3sat(vars, cls);
            
            DPLL solver;
            auto start = chrono::high_resolution_clock::now();
            bool result = solver.solve(vars, cnf);
            auto end = chrono::high_resolution_clock::now();
            double ms = chrono::duration<double, milli>(end - start).count();
            
            cout << "  " << vars << "\t" << cls << "\t" 
                 << solver.nodes << "\t" << fixed << setprecision(2) << ms << "\t"
                 << (result ? "SAT" : "UNSAT") << "\n";
        }
    }
    
    cout << "\n  ╔════════════════════════════════════════════════╗\n";
    cout << "  ║  OBSERVATIONS:                                 ║\n";
    cout << "  ║  • 3-SAT is HARD for DPLL                    ║\n";
    cout << "  ║  • Nodes grow EXPONENTIALLY                  ║\n";
    cout << "  ║  • This is why P=NP is unsolved             ║\n";
    cout << "  ║  • φ·ψ = -1 (hindi 2)                       ║\n";
    cout << "  ╚════════════════════════════════════════════════╝\n";
    
    return 0;
}
