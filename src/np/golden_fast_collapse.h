#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <queue>

namespace GoldenFastCollapse {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenFastSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    // Use integer representation para sa speed
    // 1 = TRUE (PHI), -1 = FALSE (PSI), 0 = unassigned
    using Assignment = std::vector<int>;
    
    static bool clause_satisfied_fast(const std::vector<int>& clause,
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
    
    // Fast propagation gamit ang queue
    static bool propagate_fast(const std::vector<std::vector<int>>& clauses,
                               Assignment& state,
                               std::queue<int>& to_process) {
        while (!to_process.empty()) {
            int clause_idx = to_process.front();
            to_process.pop();
            
            const auto& clause = clauses[clause_idx];
            if (clause_satisfied_fast(clause, state)) continue;
            
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
                state[var] = (last_lit > 0) ? 1 : -1;
                
                // Add affected clauses sa queue
                for (int i = 0; i < clauses.size(); i++) {
                    for (int lit : clauses[i]) {
                        if (abs(lit) - 1 == var) {
                            to_process.push(i);
                            break;
                        }
                    }
                }
            } else if (unassigned_count == 0) {
                return false;  // Conflict
            }
        }
        return true;
    }
    
    static int select_variable_fast(const std::vector<std::vector<int>>& clauses,
                                    const Assignment& state) {
        int best_var = -1;
        int best_count = -1;
        
        for (int v = 0; v < state.size(); v++) {
            if (state[v] != 0) continue;
            
            int count = 0;
            for (const auto& clause : clauses) {
                if (clause_satisfied_fast(clause, state)) continue;
                
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
    
    static bool solve_recursive(const std::vector<std::vector<int>>& clauses,
                                Assignment& state,
                                long long& steps) {
        steps++;
        
        // Initial propagation
        std::queue<int> to_process;
        for (int i = 0; i < clauses.size(); i++) {
            to_process.push(i);
        }
        
        if (!propagate_fast(clauses, state, to_process)) {
            return false;
        }
        
        // Check complete
        bool complete = true;
        for (int val : state) {
            if (val == 0) {
                complete = false;
                break;
            }
        }
        
        if (complete) {
            for (const auto& clause : clauses) {
                if (!clause_satisfied_fast(clause, state)) {
                    return false;
                }
            }
            return true;
        }
        
        int var = select_variable_fast(clauses, state);
        
        if (var == -1) return false;
        
        // Try TRUE
        Assignment state_true = state;
        state_true[var] = 1;
        if (solve_recursive(clauses, state_true, steps)) {
            state = state_true;
            return true;
        }
        
        // Try FALSE
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

} // namespace GoldenFastCollapse
