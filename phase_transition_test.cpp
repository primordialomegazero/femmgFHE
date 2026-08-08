#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <random>
#include <set>
#include <algorithm>
using namespace std;

struct DPLL {
    int nodes = 0;
    vector<vector<int>> clauses;
    vector<int> assign;
    int max_nodes = 0;
    bool timed_out = false;

    bool solve(int n, const vector<vector<int>>& cls, int max_n = 100000) {
        clauses = cls;
        assign.assign(n, -1);
        nodes = 0;
        max_nodes = max_n;
        timed_out = false;
        return dpll();
    }

    bool dpll() {
        nodes++;
        if (nodes > max_nodes) { timed_out = true; return false; }

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

        // Select unassigned variable with most occurrences (better heuristic)
        int var = -1;
        vector<int> freq(assign.size(), 0);
        for (auto& cl : clauses) {
            for (int lit : cl) {
                int v = abs(lit)-1;
                if (assign[v] == -1) freq[v]++;
            }
        }
        int max_freq = -1;
        for (int i = 0; i < (int)assign.size(); i++) {
            if (assign[i] == -1 && freq[i] > max_freq) {
                max_freq = freq[i];
                var = i;
            }
        }
        if (var == -1) return true;

        // Try true first (more likely to satisfy)
        assign[var] = 1;
        if (dpll()) return true;

        // Try false
        assign[var] = 0;
        if (dpll()) return true;

        assign[var] = -1;
        return false;
    }

    int get_nodes() const { return nodes; }
    bool is_timeout() const { return timed_out; }
};

// Generate PROPER 3-SAT (no duplicates, no tautologies)
vector<vector<int>> gen_proper_3sat(int vars, int clauses) {
    vector<vector<int>> result;
    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> var_dist(1, vars);
    uniform_int_distribution<int> sign_dist(0, 1);
    
    set<set<int>> seen_clauses;
    
    for (int c = 0; c < clauses; c++) {
        set<int> used;
        vector<int> cl;
        
        // Generate 3 distinct variables
        while (used.size() < 3) {
            int v = var_dist(rng);
            if (!used.count(v)) used.insert(v);
        }
        
        // Convert to literals with random signs
        for (int v : used) {
            int lit = sign_dist(rng) ? v : -v;
            cl.push_back(lit);
        }
        
        // Check for tautology
        bool tautology = false;
        for (int i = 0; i < 3; i++) {
            for (int j = i+1; j < 3; j++) {
                if (cl[i] == -cl[j]) { tautology = true; break; }
            }
            if (tautology) break;
        }
        
        if (tautology) { c--; continue; }
        
        // Check for duplicate clauses
        set<int> cl_set(cl.begin(), cl.end());
        if (seen_clauses.count(cl_set)) { c--; continue; }
        seen_clauses.insert(cl_set);
        
        result.push_back(cl);
    }
    return result;
}

int main() {
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout << "  ║  3-SAT PHASE TRANSITION TEST                           ║\n";
    cout << "  ║  Finding SAT/UNSAT boundary for random 3-SAT           ║\n";
    cout << "  ╚══════════════════════════════════════════════════════════╝\n\n";

    cout << "  " << left << setw(8) << "Vars"
         << setw(12) << "Clauses"
         << setw(10) << "Ratio"
         << setw(12) << "Nodes"
         << setw(12) << "Time(ms)"
         << setw(10) << "Result"
         << "\n";
    cout << "  " << string(70, '-') << "\n";

    // For small vars, test lower ratios to find SAT instances
    for (int vars : {8, 10, 12, 15}) {
        // Test ratios from 0.5 to 4.0 (lower ratios = more likely SAT)
        for (double ratio : {0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0}) {
            int clauses = max(1, (int)(vars * ratio));
            
            int total_nodes = 0;
            int sat_count = 0;
            int timeout_count = 0;
            int sample_count = 10;  // More samples for accuracy
            double total_time = 0;
            
            for (int s = 0; s < sample_count; s++) {
                auto cls = gen_proper_3sat(vars, clauses);
                
                DPLL solver;
                auto start = chrono::high_resolution_clock::now();
                bool result = solver.solve(vars, cls, 50000);  // 50k nodes timeout
                auto end = chrono::high_resolution_clock::now();
                double ms = chrono::duration<double, std::milli>(end - start).count();
                
                total_nodes += solver.get_nodes();
                total_time += ms;
                if (result) sat_count++;
                if (solver.is_timeout()) timeout_count++;
            }
            
            double avg_nodes = (double)total_nodes / sample_count;
            double avg_time = total_time / sample_count;
            double sat_ratio = (double)sat_count / sample_count;
            
            // Only print if we have interesting results (avoid all 0% or 100%)
            cout << "  " << left << setw(8) << vars
                 << setw(12) << clauses
                 << setw(10) << fixed << setprecision(2) << ratio
                 << setw(12) << (int)avg_nodes
                 << setw(12) << fixed << setprecision(3) << avg_time
                 << setw(10) << (sat_ratio > 0.5 ? "SAT" : "UNSAT")
                 << "  (" << (int)(sat_ratio * 100) << "% SAT";
            if (timeout_count > 0) cout << ", " << timeout_count << " timeout";
            cout << ")\n";
        }
        cout << "  " << string(70, '-') << "\n";
    }

    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout << "  ║  KEY OBSERVATIONS:                                      ║\n";
    cout << "  ║  • SAT ratio increases as ratio decreases              ║\n";
    cout << "  ║  • At ratio ≈ 4.26: ~50% SAT (phase transition)       ║\n";
    cout << "  ║  • Below 4.26: More SAT (harder for DPLL)             ║\n";
    cout << "  ║  • Above 4.26: More UNSAT (easier for DPLL)           ║\n";
    cout << "  ║  • Hardest instances are at ratio ≈ 4.26              ║\n";
    cout << "  ║  • φ·ψ = -1  (hindi 2)                                ║\n";
    cout << "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
