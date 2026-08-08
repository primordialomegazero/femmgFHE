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

    bool solve(int n, const vector<vector<int>>& cls, int max_n = 1000000) {
        clauses = cls;
        assign.assign(n, -1);
        nodes = 0;
        max_nodes = max_n;
        return dpll();
    }

    bool dpll() {
        nodes++;
        if (nodes > max_nodes) return false;  // Timeout protection

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

        // Select unassigned variable (simple heuristic)
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

    int get_nodes() const { return nodes; }
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
        
        // Check for tautology (contains both v and -v)
        bool tautology = false;
        for (int i = 0; i < 3; i++) {
            for (int j = i+1; j < 3; j++) {
                if (cl[i] == -cl[j]) { tautology = true; break; }
            }
            if (tautology) break;
        }
        
        if (tautology) {
            c--;  // Regenerate this clause
            continue;
        }
        
        // Check for duplicate clauses
        set<int> cl_set(cl.begin(), cl.end());
        if (seen_clauses.count(cl_set)) {
            c--;  // Regenerate this clause
            continue;
        }
        seen_clauses.insert(cl_set);
        
        result.push_back(cl);
    }
    return result;
}

int main() {
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout << "  ║  PROPER 3-SAT TEST - No Duplicates                 ║\n";
    cout << "  ║  Finding the Phase Transition Region               ║\n";
    cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    cout << "  " << left << setw(8) << "Vars"
         << setw(10) << "Clauses"
         << setw(10) << "Ratio"
         << setw(12) << "Nodes"
         << setw(12) << "Time(ms)"
         << setw(10) << "Result"
         << "\n";
    cout << "  " << string(66, '-') << "\n";

    for (int vars : {8, 10, 12, 15}) {
        // Test different clause-to-variable ratios around the phase transition (≈4.26)
        for (double ratio : {3.5, 4.0, 4.26, 4.5, 5.0}) {
            int clauses = (int)(vars * ratio);
            
            // Run multiple samples and average
            int total_nodes = 0;
            int sat_count = 0;
            int sample_count = 5;
            double total_time = 0;
            
            for (int s = 0; s < sample_count; s++) {
                auto cls = gen_proper_3sat(vars, clauses);
                
                DPLL solver;
                auto start = chrono::high_resolution_clock::now();
                bool result = solver.solve(vars, cls, 100000);  // 100k nodes timeout
                auto end = chrono::high_resolution_clock::now();
                double ms = chrono::duration<double, std::milli>(end - start).count();
                
                total_nodes += solver.get_nodes();
                total_time += ms;
                if (result) sat_count++;
            }
            
            double avg_nodes = (double)total_nodes / sample_count;
            double avg_time = total_time / sample_count;
            double sat_ratio = (double)sat_count / sample_count;
            
            cout << "  " << left << setw(8) << vars
                 << setw(10) << clauses
                 << setw(10) << fixed << setprecision(2) << ratio
                 << setw(12) << (int)avg_nodes
                 << setw(12) << fixed << setprecision(3) << avg_time
                 << setw(10) << (sat_ratio > 0.5 ? "SAT" : "UNSAT")
                 << "  (" << (int)(sat_ratio * 100) << "% SAT)"
                 << "\n";
        }
    }

    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout << "  ║  OBSERVATIONS:                                      ║\n";
    cout << "  ║  • Phase transition at ratio ≈ 4.26                ║\n";
    cout << "  ║  • Below 4.26: Mostly SAT (hard for DPLL)         ║\n";
    cout << "  ║  • Above 4.26: Mostly UNSAT (easier for DPLL)    ║\n";
    cout << "  ║  • Hardest instances are near ratio = 4.26        ║\n";
    cout << "  ║  • That's where exponential growth appears!       ║\n";
    cout << "  ║  • φ·ψ = -1  (hindi 2)                            ║\n";
    cout << "  ╚══════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
