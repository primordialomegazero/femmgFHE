#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>

namespace GoldenCached {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenCachedSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    using Assignment = std::vector<int>;  // 1=TRUE, -1=FALSE, 0=unassigned
    
    static bool is_assigned(int val) { return val != 0; }
    static bool get_value(int val) { return val == 1; }
    static int assign(bool val) { return val ? 1 : -1; }
    
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
    
    // Propagate with cached clause satisfaction
    static bool propagate_cached(const std::vector<std::vector<int>>& clauses,
                                 Assignment& state) {
        bool changed = true;
        
        while (changed) {
            changed = false;
            
            for (const auto& clause : clauses) {
                if (clause_satisfied(clause, state)) continue;
                
                int unassigned_count = 0;
                int last_lit = 0;
                
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (state[var] == 0) {
                        unassigned_count++;
                        last_lit = lit;
                    }
                }
                
                if (unassigned_count == 1) {
                    int var = abs(last_lit) - 1;
                    state[var] = assign(last_lit > 0);
                    changed = true;
                } else if (unassigned_count == 0) {
                    return false;  // Conflict
                }
            }
        }
        return true;
    }
    
    // Fast variable selection - bilangin lang ang unassigned sa unsatisfied clauses
    static int select_variable_fast(const std::vector<std::vector<int>>& clauses,
                                    const Assignment& state) {
        std::vector<int> counts(state.size(), 0);
        
        for (const auto& clause : clauses) {
            if (clause_satisfied(clause, state)) continue;
            
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (state[var] == 0) {
                    counts[var]++;
                }
            }
        }
        
        int best_var = -1;
        int best_count = -1;
        for (int v = 0; v < counts.size(); v++) {
            if (state[v] == 0 && counts[v] > best_count) {
                best_count = counts[v];
                best_var = v;
            }
        }
        
        return best_var;
    }
    
    static bool solve_recursive(const std::vector<std::vector<int>>& clauses,
                                Assignment& state,
                                long long& steps) {
        steps++;
        
        if (!propagate_cached(clauses, state)) {
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
            for (const auto& clause : clauses) {
                if (!clause_satisfied(clause, state)) return false;
            }
            return true;
        }
        
        int var = select_variable_fast(clauses, state);
        if (var == -1) return false;
        
        Assignment state_true = state;
        state_true[var] = 1;
        if (solve_recursive(clauses, state_true, steps)) {
            state = state_true;
            return true;
        }
        
        Assignment state_false = state;
        state_false[var] = -1;
        if (solve_recursive(clauses, state_false, steps)) {
            state = state_false;
            return true;
        }
        
        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        Assignment state(num_vars, 0);
        
        result.satisfiable = solve_recursive(clauses, state, result.steps);
        
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] == 1);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenCached
