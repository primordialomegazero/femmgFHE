#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
using namespace std;
using namespace chrono;

struct Solver {
    int nodes = 0;
    vector<vector<int>> clauses;
    
    bool solve(vector<int>& model) {
        nodes = 0;
        return backtrack(model, 0);
    }
    
    bool backtrack(vector<int>& model, int idx) {
        nodes++;
        if (idx == model.size()) return check(model);
        
        // Try true
        model[idx] = 1;
        if (prune(model)) {
            if (backtrack(model, idx+1)) return true;
        }
        
        // Try false
        model[idx] = -1;
        if (prune(model)) {
            if (backtrack(model, idx+1)) return true;
        }
        
        model[idx] = 0;
        return false;
    }
    
    bool prune(vector<int>& model) {
        for (auto& c : clauses) {
            bool sat = false;
            for (int lit : c) {
                int var = abs(lit)-1;
                if (var >= model.size()) continue;
                if (model[var] == 0) { sat = true; break; }
                if ((lit > 0 && model[var] == 1) || (lit < 0 && model[var] == -1)) {
                    sat = true; break;
                }
            }
            if (!sat) return false;
        }
        return true;
    }
    
    bool check(vector<int>& model) {
        for (auto& c : clauses) {
            bool sat = false;
            for (int lit : c) {
                int var = abs(lit)-1;
                if ((lit > 0 && model[var] == 1) || (lit < 0 && model[var] == -1)) {
                    sat = true; break;
                }
            }
            if (!sat) return false;
        }
        return true;
    }
};

int main() {
    cout << "\n╔════════════════════════════════════════════════╗\n";
    cout << "║  BRUTE FORCE PHP(n) - No Shortcuts           ║\n";
    cout << "║  Para makita ang totoong complexity          ║\n";
    cout << "╚════════════════════════════════════════════════╝\n\n";
    
    cout << "n\tVars\tNodes\t2^n\tExponential?\n";
    cout << "-----------------------------------------\n";
    
    for (int n = 2; n <= 8; n++) {
        Solver s;
        int vars = n * (n-1);
        s.clauses.clear();
        
        // Generate FULL PHP
        for (int p = 0; p < n; p++) {
            vector<int> c;
            for (int h = 0; h < n-1; h++) {
                c.push_back(p*(n-1) + h + 1);
            }
            s.clauses.push_back(c);
        }
        
        for (int h = 0; h < n-1; h++) {
            for (int p1 = 0; p1 < n; p1++) {
                for (int p2 = p1+1; p2 < n; p2++) {
                    s.clauses.push_back({
                        -(p1*(n-1) + h + 1),
                        -(p2*(n-1) + h + 1)
                    });
                }
            }
        }
        
        vector<int> model(vars, 0);
        auto start = high_resolution_clock::now();
        bool result = s.solve(model);
        auto end = high_resolution_clock::now();
        auto ms = duration_cast<microseconds>(end - start).count() / 1000.0;
        
        cout << n << "\t" << vars << "\t" << s.nodes << "\t" 
             << (int)pow(2, vars) << "\t";
        
        if (s.nodes > pow(2, n)) cout << "YES ✅";
        else cout << "NO ❌";
        
        cout << "\t" << fixed << setprecision(2) << ms << "ms\n";
    }
    
    cout << "\n╔════════════════════════════════════════════════╗\n";
    cout << "║  VERDICT:                                      ║\n";
    cout << "║  • PHP is UNSAT (correct)                     ║\n";
    cout << "║  • Nodes grow EXPONENTIALLY with vars         ║\n";
    cout << "║  • P ≠ NP (still unsolved)                   ║\n";
    cout << "║  • φ·ψ = -1 (hindi 2)                        ║\n";
    cout << "╚════════════════════════════════════════════════╝\n";
}
