#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>

namespace GoldenCompleteOptimized {

constexpr double PHI = 1.6180339887498948482;

class GoldenCompleteSolver {
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
    
    static bool all_satisfied(const std::vector<std::vector<int>>& clauses,
                             const std::vector<int>& assignment) {
        for (const auto& clause : clauses) {
            if (!is_clause_satisfied(clause, assignment)) {
                return false;
            }
        }
        return true;
    }
    
    static bool has_conflict(const std::vector<std::vector<int>>& clauses,
                            const std::vector<int>& assignment) {
        for (const auto& clause : clauses) {
            bool has_unassigned = false;
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (assignment[var] == -1) {
                    has_unassigned = true;
                    break;
                }
            }
            if (!has_unassigned && !is_clause_satisfied(clause, assignment)) {
                return true;
            }
        }
        return false;
    }
    
    // Golden ratio variable selection - pero COMPLETE pa rin
    static int select_variable(const std::vector<std::vector<int>>& clauses,
                              const std::vector<int>& assignment,
                              int num_vars) {
        int best_var = -1;
        double best_score = -1e18;
        
        for (int v = 0; v < num_vars; v++) {
            if (assignment[v] != -1) continue;
            
            int pos_count = 0, neg_count = 0;
            
            for (const auto& clause : clauses) {
                if (is_clause_satisfied(clause, assignment)) continue;
                
                for (int lit : clause) {
                    if (abs(lit) - 1 == v) {
                        if (lit > 0) pos_count++;
                        else neg_count++;
                    }
                }
            }
            
            int total = pos_count + neg_count;
            if (total > 0) {
                double balance = std::abs(pos_count - neg_count) / (double)total;
                // Golden ratio balance - mas balanced mas maganda
                double score = total / (1.0 + balance * PHI);
                
                if (score > best_score) {
                    best_score = score;
                    best_var = v;
                }
            }
        }
        
        // Fallback: first unassigned kung walang mahanap
        if (best_var == -1) {
            for (int i = 0; i < num_vars; i++) {
                if (assignment[i] == -1) {
                    best_var = i;
                    break;
                }
            }
        }
        
        return best_var;
    }
    
    static bool dpll_recursive(const std::vector<std::vector<int>>& clauses,
                              std::vector<int>& assignment,
                              long long& steps,
                              long long& backtracks) {
        steps++;
        
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
        
        // Check for conflict
        if (has_conflict(clauses, assignment)) {
            backtracks++;
            return false;
        }
        
        // Select variable (golden ratio guided)
        int var = select_variable(clauses, assignment, assignment.size());
        
        if (var == -1) return false;
        
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

} // namespace GoldenCompleteOptimized
