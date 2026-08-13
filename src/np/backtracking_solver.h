#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>

namespace BacktrackingSolver {

class DPLLSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long backtracks;
        double time_ms;
    };

private:
    static bool check_clause(const std::vector<int>& clause, const std::vector<int>& assignment) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            if (var < assignment.size()) {
                bool val = assignment[var] > 0;
                int assigned_val = assignment[var];
                if ((lit > 0 && assigned_val == 1) || (lit < 0 && assigned_val == 0)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    static bool propagate(std::vector<std::vector<int>>& clauses, 
                         std::vector<int>& assignment, int& num_assigned) {
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (auto& clause : clauses) {
                int unassigned_count = 0;
                int last_unassigned = -1;
                bool clause_satisfied = false;
                
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (var < assignment.size() && assignment[var] != -1) {
                        bool val = assignment[var] == 1;
                        if ((lit > 0 && val) || (lit < 0 && !val)) {
                            clause_satisfied = true;
                            break;
                        }
                    } else {
                        unassigned_count++;
                        last_unassigned = lit;
                    }
                }
                
                if (!clause_satisfied && unassigned_count == 1) {
                    // Unit propagation
                    int var = abs(last_unassigned) - 1;
                    assignment[var] = (last_unassigned > 0) ? 1 : 0;
                    num_assigned++;
                    changed = true;
                } else if (!clause_satisfied && unassigned_count == 0) {
                    // Conflict
                    return false;
                }
            }
        }
        return true;
    }
    
    static bool dpll_recursive(std::vector<std::vector<int>> clauses, 
                              std::vector<int>& assignment, 
                              int num_assigned, int num_vars,
                              long long& steps, long long& backtracks) {
        steps++;
        
        // Propagate unit clauses
        if (!propagate(clauses, assignment, num_assigned)) {
            backtracks++;
            return false;
        }
        
        // Check if complete assignment
        if (num_assigned == num_vars) {
            // Verify all clauses satisfied
            for (const auto& clause : clauses) {
                if (!check_clause(clause, assignment)) {
                    backtracks++;
                    return false;
                }
            }
            return true;
        }
        
        // Choose next variable (MRV heuristic - Most Restricted Variable)
        int best_var = -1;
        int best_score = -1;
        
        for (int v = 0; v < num_vars; v++) {
            if (assignment[v] == -1) {
                // Count occurrences in clauses
                int score = 0;
                for (const auto& clause : clauses) {
                    for (int lit : clause) {
                        if (abs(lit) - 1 == v) {
                            score++;
                        }
                    }
                }
                if (score > best_score) {
                    best_score = score;
                    best_var = v;
                }
            }
        }
        
        if (best_var == -1) {
            return num_assigned == num_vars;
        }
        
        // Try TRUE first
        assignment[best_var] = 1;
        if (dpll_recursive(clauses, assignment, num_assigned + 1, num_vars, 
                          steps, backtracks)) {
            return true;
        }
        
        // Try FALSE
        assignment[best_var] = 0;
        if (dpll_recursive(clauses, assignment, num_assigned + 1, num_vars, 
                          steps, backtracks)) {
            return true;
        }
        
        // Backtrack
        assignment[best_var] = -1;
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
        
        // Initialize assignment (-1 = unassigned)
        std::vector<int> assignment(num_vars, -1);
        int num_assigned = 0;
        
        result.satisfiable = dpll_recursive(clauses, assignment, num_assigned, 
                                           num_vars, result.steps, result.backtracks);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        // Convert to bool assignment
        result.assignment.resize(num_vars);
        for (int i = 0; i < num_vars; i++) {
            result.assignment[i] = (assignment[i] == 1);
        }
        
        return result;
    }
};

} // namespace BacktrackingSolver
