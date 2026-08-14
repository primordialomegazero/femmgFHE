#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>

namespace ProperDPLL {

class ProperDPLLSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long backtracks;
        double time_ms;
    };

private:
    static bool is_clause_satisfied(const std::vector<int>& clause,
                                    const std::vector<int>& assignment) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            if (assignment[var] != -1) {
                bool val = assignment[var] == 1;
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    // Unit propagation - critical para sa efficiency
    static bool unit_propagate(const std::vector<std::vector<int>>& clauses,
                              std::vector<int>& assignment) {
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (const auto& clause : clauses) {
                if (is_clause_satisfied(clause, assignment)) continue;
                
                int unassigned_count = 0;
                int last_unassigned = -1;
                
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (assignment[var] == -1) {
                        unassigned_count++;
                        last_unassigned = lit;
                    }
                }
                
                if (unassigned_count == 0) {
                    return false;  // Conflict
                } else if (unassigned_count == 1) {
                    // Unit clause: force assignment
                    int var = abs(last_unassigned) - 1;
                    assignment[var] = (last_unassigned > 0) ? 1 : 0;
                    changed = true;
                }
            }
        }
        return true;
    }
    
    static bool all_satisfied(const std::vector<std::vector<int>>& clauses,
                             const std::vector<int>& assignment) {
        for (const auto& clause : clauses) {
            if (!is_clause_satisfied(clause, assignment)) {
                return false;
            }
        }
        return true;
    }
    
    // Better variable selection - Most Constrained Variable
    static int select_variable(const std::vector<std::vector<int>>& clauses,
                              const std::vector<int>& assignment,
                              int num_vars) {
        int best_var = -1;
        int best_count = -1;
        
        for (int v = 0; v < num_vars; v++) {
            if (assignment[v] != -1) continue;
            
            int count = 0;
            for (const auto& clause : clauses) {
                if (is_clause_satisfied(clause, assignment)) continue;
                
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
    
    static bool dpll_recursive(const std::vector<std::vector<int>>& clauses,
                              std::vector<int>& assignment,
                              long long& steps,
                              long long& backtracks) {
        steps++;
        
        // Unit propagation - CRITICAL!
        if (!unit_propagate(clauses, assignment)) {
            backtracks++;
            return false;
        }
        
        // Check if complete
        bool complete = true;
        for (int val : assignment) {
            if (val == -1) {
                complete = false;
                break;
            }
        }
        
        if (complete) {
            return all_satisfied(clauses, assignment);
        }
        
        // Select variable
        int var = select_variable(clauses, assignment, assignment.size());
        
        if (var == -1) {
            return all_satisfied(clauses, assignment);
        }
        
        // Try TRUE
        assignment[var] = 1;
        if (dpll_recursive(clauses, assignment, steps, backtracks)) {
            return true;
        }
        
        // Try FALSE
        assignment[var] = 0;
        if (dpll_recursive(clauses, assignment, steps, backtracks)) {
            return true;
        }
        
        // Backtrack
        assignment[var] = -1;
        backtracks++;
        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.backtracks = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<int> assignment(num_vars, -1);
        
        result.satisfiable = dpll_recursive(clauses, assignment, 
                                           result.steps, result.backtracks);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        result.assignment.resize(num_vars);
        for (int i = 0; i < num_vars; i++) {
            result.assignment[i] = (assignment[i] == 1);
        }
        
        return result;
    }
};

} // namespace ProperDPLL
