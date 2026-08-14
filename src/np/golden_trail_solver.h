#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>

namespace GoldenTrail {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenTrailSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long backtracks;
        double time_ms;
    };

private:
    using Assignment = std::vector<int>;  // 1=TRUE, -1=FALSE, 0=unassigned
    
    // Trail entry: ano ang in-assign at bakit
    struct TrailEntry {
        int var;
        int value;
        bool is_decision;  // true = decision, false = propagation
        
        TrailEntry(int v, int val, bool dec) : var(v), value(val), is_decision(dec) {}
    };
    
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
    
    // Incremental propagation - check lang ang clauses na may bagong assignment
    static bool propagate_incremental(const std::vector<std::vector<int>>& clauses,
                                      Assignment& state,
                                      std::vector<TrailEntry>& trail) {
        bool changed = true;
        
        while (changed) {
            changed = false;
            
            for (const auto& clause : clauses) {
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
                    trail.push_back(TrailEntry(var, state[var], false));
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
    
    // Backtrack sa last decision
    static void backtrack_to_last_decision(Assignment& state,
                                           std::vector<TrailEntry>& trail) {
        while (!trail.empty()) {
            TrailEntry entry = trail.back();
            trail.pop_back();
            state[entry.var] = 0;
            
            if (entry.is_decision) {
                break;
            }
        }
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.backtracks = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        Assignment state(num_vars, 0);
        std::vector<TrailEntry> trail;
        std::vector<int> decision_stack;
        
        while (true) {
            result.steps++;
            
            // Propagate
            if (propagate_incremental(clauses, state, trail)) {
                // Check kung complete
                bool complete = true;
                for (int val : state) {
                    if (val == 0) {
                        complete = false;
                        break;
                    }
                }
                
                if (complete) {
                    result.satisfiable = true;
                    break;
                }
                
                // Select variable
                int var = select_variable(state, clauses);
                
                if (var == -1) {
                    // Walang ma-assign - backtrack
                    if (decision_stack.empty()) break;
                    backtrack_to_last_decision(state, trail);
                    decision_stack.pop_back();
                    result.backtracks++;
                    continue;
                }
                
                // Make decision
                state[var] = 1;  // Try TRUE
                trail.push_back(TrailEntry(var, 1, true));
                decision_stack.push_back(var);
            } else {
                // Conflict - backtrack
                if (decision_stack.empty()) break;
                backtrack_to_last_decision(state, trail);
                decision_stack.pop_back();
                result.backtracks++;
            }
        }
        
        // Extract assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] == 1);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenTrail
