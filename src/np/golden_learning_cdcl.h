#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace GoldenCDCL {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenCDCLSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long conflicts;
        long long learned_clauses;
        double time_ms;
    };

private:
    using Assignment = std::vector<int>;  // 1=TRUE, -1=FALSE, 0=unassigned
    
    static bool clause_satisfied(const std::vector<int>& clause,
                                 const Assignment& state) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            if (state[var] == 0) continue;
            bool val = state[var] == 1;
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                return true;
            }
        }
        return false;
    }
    
    static bool propagate(Assignment& state,
                          const std::vector<std::vector<int>>& clauses,
                          std::vector<int>& trail) {
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (int ci = 0; ci < clauses.size(); ci++) {
                const auto& clause = clauses[ci];
                if (clause_satisfied(clause, state)) continue;
                
                int unassigned = 0;
                int last_lit = 0;
                
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (state[var] == 0) {
                        unassigned++;
                        last_lit = lit;
                    }
                }
                
                if (unassigned == 1) {
                    int var = abs(last_lit) - 1;
                    state[var] = (last_lit > 0) ? 1 : -1;
                    trail.push_back(var);
                    changed = true;
                } else if (unassigned == 0) {
                    return false;  // Conflict
                }
            }
        }
        return true;
    }
    
    static int select_variable(const Assignment& state,
                               const std::vector<std::vector<int>>& clauses) {
        int best_var = -1;
        int best_count = -1;
        
        for (int v = 0; v < state.size(); v++) {
            if (state[v] != 0) continue;
            
            int count = 0;
            for (const auto& clause : clauses) {
                if (clause_satisfied(clause, state)) continue;
                for (int lit : clause) {
                    if (abs(lit) - 1 == v) {
                        count++;
                        break;
                    }
                }
            }
            
            if (count > best_count) {
                best_count = count;
                best_var = v;
            }
        }
        
        return best_var;
    }
    
    // Analyze conflict at gumawa ng learned clause
    static std::vector<int> analyze_conflict(const std::vector<std::vector<int>>& clauses,
                                             const Assignment& state) {
        // Hanapin ang mga false literals sa unsatisfied clauses
        std::vector<int> learned;
        
        for (const auto& clause : clauses) {
            if (clause_satisfied(clause, state)) continue;
            
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (state[var] != 0) {
                    // False literal - i-negate para sa learned clause
                    learned.push_back(-lit);
                }
            }
        }
        
        return learned;
    }
    
    static bool solve_recursive(std::vector<std::vector<int>>& clauses,
                                Assignment& state,
                                long long& steps,
                                long long& conflicts,
                                long long& learned_clauses) {
        steps++;
        
        std::vector<int> trail;
        if (!propagate(state, clauses, trail)) {
            conflicts++;
            
            // Conflict analysis
            std::vector<int> learned = analyze_conflict(clauses, state);
            if (!learned.empty()) {
                clauses.push_back(learned);
                learned_clauses++;
            }
            
            return false;
        }
        
        bool complete = true;
        for (int val : state) {
            if (val == 0) {
                complete = false;
                break;
            }
        }
        
        if (complete) {
            return true;
        }
        
        int var = select_variable(state, clauses);
        if (var == -1) return false;
        
        Assignment state_true = state;
        state_true[var] = 1;
        if (solve_recursive(clauses, state_true, steps, conflicts, learned_clauses)) {
            state = state_true;
            return true;
        }
        
        Assignment state_false = state;
        state_false[var] = -1;
        if (solve_recursive(clauses, state_false, steps, conflicts, learned_clauses)) {
            state = state_false;
            return true;
        }
        
        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses_input, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.conflicts = 0;
        result.learned_clauses = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::vector<int>> clauses = clauses_input;
        Assignment state(num_vars, 0);
        
        result.satisfiable = solve_recursive(clauses, state, result.steps,
                                             result.conflicts, result.learned_clauses);
        
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] == 1);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenCDCL
