#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>

namespace GoldenPropagatingOptimized {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenCollapseSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    static bool is_assigned(double val) {
        return std::abs(val - PHI) < 1e-6 || std::abs(val - PSI) < 1e-6;
    }
    
    static bool get_value(double val) {
        return val > 0;
    }
    
    static double assign(bool val) {
        return val ? PHI : PSI;
    }
    
    // Optimized clause satisfaction check
    static bool clause_satisfied(const std::vector<int>& clause,
                                 const std::vector<double>& state) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            if (!is_assigned(state[var])) continue;
            
            bool val = get_value(state[var]);
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                return true;
            }
        }
        return false;
    }
    
    // Optimized propagation - may early exit
    static bool propagate_collapse(const std::vector<std::vector<int>>& clauses,
                                   std::vector<double>& state) {
        bool changed = true;
        
        while (changed) {
            changed = false;
            
            for (const auto& clause : clauses) {
                if (clause_satisfied(clause, state)) continue;
                
                int unassigned_count = 0;
                int last_unassigned_lit = 0;
                
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (!is_assigned(state[var])) {
                        unassigned_count++;
                        last_unassigned_lit = lit;
                    }
                }
                
                if (unassigned_count == 1) {
                    // Unit clause - collapse
                    int var = abs(last_unassigned_lit) - 1;
                    state[var] = assign(last_unassigned_lit > 0);
                    changed = true;
                } else if (unassigned_count == 0) {
                    return false;  // Conflict
                }
            }
        }
        
        return true;
    }
    
    // Optimized variable selection - may activity tracking
    static int select_variable(const std::vector<std::vector<int>>& clauses,
                              const std::vector<double>& state) {
        int best_var = -1;
        int best_count = -1;
        
        for (int v = 0; v < state.size(); v++) {
            if (is_assigned(state[v])) continue;
            
            int count = 0;
            for (const auto& clause : clauses) {
                if (clause_satisfied(clause, state)) continue;
                
                for (int lit : clause) {
                    if (abs(lit) - 1 == v) {
                        count++;
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
    
    static bool collapse_recursive(const std::vector<std::vector<int>>& clauses,
                                   std::vector<double>& state,
                                   long long& steps) {
        steps++;
        
        if (!propagate_collapse(clauses, state)) {
            return false;
        }
        
        bool complete = true;
        for (double val : state) {
            if (!is_assigned(val)) {
                complete = false;
                break;
            }
        }
        
        if (complete) {
            for (const auto& clause : clauses) {
                if (!clause_satisfied(clause, state)) {
                    return false;
                }
            }
            return true;
        }
        
        int var = select_variable(clauses, state);
        
        if (var == -1) {
            return false;
        }
        
        // Collapse sa TRUE
        std::vector<double> state_true = state;
        state_true[var] = assign(true);
        if (collapse_recursive(clauses, state_true, steps)) {
            state = state_true;
            return true;
        }
        
        // Collapse sa FALSE
        std::vector<double> state_false = state;
        state_false[var] = assign(false);
        if (collapse_recursive(clauses, state_false, steps)) {
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
        
        std::vector<double> state(num_vars, 0.0);
        
        result.satisfiable = collapse_recursive(clauses, state, result.steps);
        
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = get_value(state[v]);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenPropagatingOptimized
