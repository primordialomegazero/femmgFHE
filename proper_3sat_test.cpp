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
        
        int var = -1;
        for (int i = 0; i < (int)assign.size(); i++) {
            if (assign[i] == -1) { var = i; break; }
        }
        if (var == -1) return true;
        
        assign[var] = 1;
        if (dpll()) return true;
        
        assign[var] = 0;
        if (dpll()) return true;
        
        assign[var] = -1;
        return false;
    }
};

vector<vector<int>> gen_3sat(int vars, int clauses, unsigned seed = 42) {
    vector<vector<int>> cls;
    mt19937 rng(seed);
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
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout << "  ║  PROPER 3-SAT TEST - Finding the Hard Region       ║\n";
    cout << "  ║  Varying clause-to-variable ratio                  ║\n";
    cout << "  ╚══════════════════════════════════════════════════════╝\n\n";
    
    cout << "  Vars\tRatio\tClauses\tNodes\tTime(ms)\tResult\n";
    cout << "  " << string(65, '-') << "\n";
    
    for (int vars = 10; vars <= 20; vars += 5) {
        for (double ratio : {1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0}) {
            int clauses = (int)(vars * ratio);
            auto cnf = gen_3sat(vars, clauses, vars * 100 + (int)(ratio * 10));
            
            DPLL solver;
            auto start = chrono::high_resolution_clock::now();
            bool result = solver.solve(vars, cnf);
            auto end = chrono::high_resolution_clock::now();
            double ms = chrono::duration<double, milli>(end - start).count();
            
            cout << "  " << vars << "\t" << fixed << setprecision(1) << ratio << "\t" 
                 << clauses << "\t" << solver.nodes << "\t" 
                 << fixed << setprecision(2) << ms << "\t"
                 << (result ? "SAT" : "UNSAT") << "\n";
        }
        cout << "  " << string(65, '-') << "\n";
    }
    
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout << "  ║  KEY OBSERVATIONS:                                   ║\n";
    cout << "  ║  • Ratio < 4.26: Mostly SAT (harder for DPLL)      ║\n";
    cout << "  ║  • Ratio > 4.26: Mostly UNSAT (easier for DPLL)    ║\n";
    cout << "  ║  • The HARDEST instances are near ratio = 4.26     ║\n";
    cout << "  ║  • That's where exponential growth appears!        ║\n";
    cout << "  ║  • φ·ψ = -1 (hindi 2)                              ║\n";
    cout << "  ╚══════════════════════════════════════════════════════╝\n";
    
    return 0;
}
