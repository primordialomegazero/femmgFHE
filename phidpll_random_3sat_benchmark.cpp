// ═══════════════════════════════════════════════════════════════
// φ-DPLL RANDOM 3-SAT SCALING BENCHMARK
// ═══════════════════════════════════════════════════════════════
// Tests whether φ-DPLL's sub-linear scaling holds on
// UNSTRUCTURED random 3-SAT instances.
// This is the bridge to ECDLP — ECDLP→SAT is unstructured.

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <map>
#include <chrono>
#include <algorithm>
#include <random>

const double PHI = 1.6180339887498948482;

// ═══════════════════════════════════════════
// φ-DPLL SOLVER (same as test_phi_no_memo)
// ═══════════════════════════════════════════
struct PhiDPLL {
    std::vector<std::vector<int>> clauses;
    int n_vars;
    long long nodes = 0;
    long long conflicts = 0;
    long long bcp_assignments = 0;

    int select_variable(const std::vector<int>& active_ids,
                        const std::map<int, int>& assign) {
        // φ-weighted: count occurrences, apply golden ratio weight
        std::vector<double> scores(n_vars + 1, 0.0);
        
        for (int cid : active_ids) {
            const auto& c = clauses[cid];
            int unassigned = 0;
            for (int lit : c) {
                int var = std::abs(lit);
                if (assign.find(var) == assign.end()) {
                    unassigned++;
                }
            }
            if (unassigned == 0) continue;
            
            double weight = 1.0 / std::pow(PHI, unassigned - 1);
            for (int lit : c) {
                int var = std::abs(lit);
                if (assign.find(var) == assign.end()) {
                    scores[var] += weight;
                }
            }
        }
        
        int best_var = -1;
        double best_score = -1;
        for (int v = 1; v <= n_vars; v++) {
            if (assign.find(v) == assign.end() && scores[v] > best_score) {
                best_score = scores[v];
                best_var = v;
            }
        }
        return best_var;
    }

    bool bcp(std::vector<int>& active_ids, std::map<int, int>& assign) {
        bool changed = true;
        while (changed) {
            changed = false;
            std::vector<int> new_active;
            for (int cid : active_ids) {
                const auto& c = clauses[cid];
                int unassigned = 0, unassigned_lit = 0;
                bool satisfied = false;
                for (int lit : c) {
                    int var = std::abs(lit);
                    auto it = assign.find(var);
                    if (it != assign.end()) {
                        if ((lit > 0 && it->second == 1) || 
                            (lit < 0 && it->second == -1)) {
                            satisfied = true; break;
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
                    int var = std::abs(unassigned_lit);
                    assign[var] = (unassigned_lit > 0) ? 1 : -1;
                    bcp_assignments++;
                    changed = true;
                } else {
                    new_active.push_back(cid);
                }
            }
            active_ids = new_active;
        }
        return true;
    }

    bool solve(std::vector<int> active_ids, std::map<int, int>& assign) {
        nodes++;
        
        if (!bcp(active_ids, assign)) return false;
        
        // Check if all clauses satisfied
        bool all_sat = true;
        for (int cid : active_ids) {
            const auto& c = clauses[cid];
            bool sat = false;
            for (int lit : c) {
                int var = std::abs(lit);
                auto it = assign.find(var);
                if (it != assign.end()) {
                    if ((lit > 0 && it->second == 1) || 
                        (lit < 0 && it->second == -1)) {
                        sat = true; break;
                    }
                }
            }
            if (!sat) { all_sat = false; break; }
        }
        if (all_sat) return true;
        
        int var = select_variable(active_ids, assign);
        if (var == -1) return true;  // all assigned
        
        // Try TRUE first
        auto assign_true = assign;
        assign_true[var] = 1;
        if (solve(active_ids, assign_true)) {
            assign = assign_true;
            return true;
        }
        
        // Try FALSE
        assign[var] = -1;
        return solve(active_ids, assign);
    }
};

// ═══════════════════════════════════════════
// RANDOM 3-SAT GENERATOR
// ═══════════════════════════════════════════
std::vector<std::vector<int>> generate_random_3sat(int n_vars, int n_clauses, int seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> var_dist(1, n_vars);
    std::uniform_int_distribution<int> sign_dist(0, 1);
    
    std::vector<std::vector<int>> clauses;
    for (int i = 0; i < n_clauses; i++) {
        std::vector<int> clause;
        for (int j = 0; j < 3; j++) {
            int var = var_dist(rng);
            int sign = sign_dist(rng) ? 1 : -1;
            clause.push_back(sign * var);
        }
        clauses.push_back(clause);
    }
    return clauses;
}

// ═══════════════════════════════════════════
// MAIN BENCHMARK
// ═══════════════════════════════════════════
int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  φ-DPLL RANDOM 3-SAT SCALING BENCHMARK                     ║\n";
    std::cout << "║  Tests: 20, 30, 40, 50, 75, 100, 150, 200 variables       ║\n";
    std::cout << "║  Clause-to-variable ratio: 4.2 (phase transition region)   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::vector<int> var_counts = {20, 30, 40, 50, 75, 100, 150, 200};
    int trials_per_size = 10;
    
    std::cout << std::left << std::setw(6) << "Vars"
              << std::setw(10) << "Clauses"
              << std::setw(10) << "Trial"
              << std::setw(12) << "Nodes"
              << std::setw(12) << "Time(ms)"
              << std::setw(10) << "Result"
              << std::setw(20) << "Brute=2^V"
              << "\n";
    std::cout << std::string(80, '-') << "\n";
    
    for (int n_vars : var_counts) {
        int n_clauses = (int)(n_vars * 4.2);
        
        long long total_nodes = 0;
        double total_time = 0;
        int sat_count = 0, unsat_count = 0;
        
        for (int trial = 0; trial < trials_per_size; trial++) {
            int seed = n_vars * 1000 + trial;
            
            PhiDPLL solver;
            solver.clauses = generate_random_3sat(n_vars, n_clauses, seed);
            solver.n_vars = n_vars;
            
            std::vector<int> active_ids;
            for (int i = 0; i < solver.clauses.size(); i++) {
                active_ids.push_back(i);
            }
            
            auto start = std::chrono::high_resolution_clock::now();
            std::map<int, int> assign;
            bool result = solver.solve(active_ids, assign);
            auto end = std::chrono::high_resolution_clock::now();
            
            double ms = std::chrono::duration<double, std::milli>(end - start).count();
            
            total_nodes += solver.nodes;
            total_time += ms;
            if (result) sat_count++; else unsat_count++;
            
            // Show each trial for smaller sizes, summary for larger
            if (n_vars <= 50) {
                std::cout << std::left << std::setw(6) << n_vars
                          << std::setw(10) << n_clauses
                          << std::setw(10) << trial
                          << std::setw(12) << solver.nodes
                          << std::setw(12) << std::fixed << std::setprecision(2) << ms
                          << std::setw(10) << (result ? "SAT" : "UNSAT")
                          << std::setw(20) << (result ? "—" : std::to_string(1LL << n_vars))
                          << "\n";
            }
        }
        
        double avg_nodes = (double)total_nodes / trials_per_size;
        double avg_time = total_time / trials_per_size;
        double brute_force = std::pow(2.0, n_vars);
        double reduction = brute_force / avg_nodes;
        
        // Show summary
        if (n_vars > 50 || true) {
            std::cout << "\n  📊 SUMMARY for " << n_vars << " vars:"
                      << " avg_nodes=" << std::fixed << std::setprecision(1) << avg_nodes
                      << " avg_time=" << std::setprecision(2) << avg_time << "ms"
                      << " SAT=" << sat_count << "/" << trials_per_size
                      << " speedup=" << std::scientific << std::setprecision(1) << reduction << "x"
                      << "\n\n";
        }
        
        // Check if too slow — stop early
        if (avg_time > 5000 && n_vars > 50) {
            std::cout << "  ⚠️  TIMEOUT THRESHOLD — stopping benchmark\n";
            break;
        }
    }
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  IF φ-DPLL IS POLYNOMIAL ON RANDOM 3-SAT:                  ║\n";
    std::cout << "║  → ECDLP→SAT encoding is feasible                         ║\n";
    std::cout << "║  → Satoshi key recovery via φ-DPLL is possible             ║\n";
    std::cout << "║  IF φ-DPLL IS EXPONENTIAL ON RANDOM 3-SAT:                 ║\n";
    std::cout << "║  → Advantage is limited to structured problems             ║\n";
    std::cout << "║  → Back to cryptanalysis for Satoshi key                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    
    return 0;
}
