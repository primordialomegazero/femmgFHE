#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <cstring>

using namespace std;
using namespace std::chrono;

vector<vector<int>> clauses;
vector<int> assignment;
long long propagations = 0;
long long conflicts = 0;
int n_vars = 0;

bool BCP() {
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& c : clauses) {
            int unassigned = 0;
            int unassigned_lit = 0;
            bool satisfied = false;
            
            for (int lit : c) {
                int var = abs(lit);
                int val = assignment[var];
                if (val != 0) {
                    if ((lit > 0 && val == 1) || (lit < 0 && val == -1)) {
                        satisfied = true;
                        break;
                    }
                } else {
                    unassigned++;
                    unassigned_lit = lit;
                }
            }
            
            if (satisfied) continue;
            if (unassigned == 0) {
                conflicts++;
                return false;
            }
            if (unassigned == 1) {
                int var = abs(unassigned_lit);
                assignment[var] = (unassigned_lit > 0) ? 1 : -1;
                propagations++;
                changed = true;
            }
        }
    }
    return true;
}

bool DPLL() {
    if (!BCP()) return false;
    
    // Find unassigned variable
    int var = 0;
    for (int v = 1; v <= n_vars; v++) {
        if (assignment[v] == 0) {
            var = v;
            break;
        }
    }
    
    // All assigned = SAT
    if (var == 0) return true;
    
    // Save state
    vector<int> saved = assignment;
    
    // Try var = 1
    assignment[var] = 1;
    propagations++;
    if (DPLL()) return true;
    
    // Restore and try var = -1
    assignment = saved;
    assignment[var] = -1;
    propagations++;
    if (DPLL()) return true;
    
    // Restore and backtrack
    assignment = saved;
    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    string line;
    int n_clauses = 0;
    
    // Parse DIMACS CNF
    while (getline(cin, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            char p, cnf[4];
            stringstream ss(line);
            ss >> p >> cnf >> n_vars >> n_clauses;
            assignment.resize(n_vars + 1, 0);
            continue;
        }
        
        vector<int> clause;
        stringstream ss(line);
        int lit;
        while (ss >> lit) {
            if (lit == 0) break;
            clause.push_back(lit);
        }
        if (!clause.empty()) {
            clauses.push_back(clause);
        }
    }
    
    if (n_vars == 0) {
        cerr << "ERROR: No DIMACS header found" << endl;
        return 1;
    }
    
    cerr << "Parsed: " << n_vars << " vars, " << clauses.size() << " clauses" << endl;
    
    auto start = high_resolution_clock::now();
    bool result = DPLL();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start).count();
    
    cout << "==============================" << endl;
    cout << "RESULT: " << (result ? "SAT" : "UNSAT") << endl;
    cout << "Vars: " << n_vars << endl;
    cout << "Clauses: " << clauses.size() << endl;
    cout << "Time: " << duration << " μs" << endl;
    cout << "Propagations: " << propagations << endl;
    cout << "Conflicts: " << conflicts << endl;
    cout << "==============================" << endl;
    
    return 0;
}
