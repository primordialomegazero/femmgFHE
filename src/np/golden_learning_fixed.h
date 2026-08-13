#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>

namespace GoldenLearningFixed {

constexpr double PHI = 1.6180339887498948482;

class GoldenLearningSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long backtracks;
        double time_ms;
        double golden_score;
    };

private:
    // Check if clause is satisfied under current assignment
    static bool clause_satisfied(const std::vector<int>& clause, 
                                 const std::vector<int>& assignment) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            if (var < assignment.size() && assignment[var] != -1) {
                bool val = assignment[var] == 1;
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    // Unit propagation - returns false on conflict
    static bool unit_propagate(std::vector<std::vector<int>>& clauses,
                              std::vector<int>& assignment,
                              int& num_assigned) {
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (auto& clause : clauses) {
                if (clause_satisfied(clause, assignment)) continue;
                
                int unassigned_count = 0;
                int last_unassigned_lit = 0;
                
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (var < assignment.size() && assignment[var] == -1) {
                        unassigned_count++;
                        last_unassigned_lit = lit;
                    }
                }
                
                if (unassigned_count == 0) {
                    return false;  // Conflict!
                } else if (unassigned_count == 1) {
                    // Unit clause found
                    int var = abs(last_unassigned_lit) - 1;
                    assignment[var] = (last_unassigned_lit > 0) ? 1 : 0;
                    num_assigned++;
                    changed = true;
                }
            }
        }
        return true;
    }
    
    // Check if all clauses satisfied
    static bool all_satisfied(const std::vector<std::vector<int>>& clauses,
                             const std::vector<int>& assignment) {
        for (const auto& clause : clauses) {
            if (!clause_satisfied(clause, assignment)) {
                return false;
            }
        }
        return true;
    }
    
    // Select variable using golden ratio heuristic
    static int select_variable(const std::vector<std::vector<int>>& clauses,
                              const std::vector<int>& assignment,
                              int num_vars) {
        int best_var = -1;
        double best_score = -1e18;
        
        for (int v = 0; v < num_vars; v++) {
            if (assignment[v] != -1) continue;
            
            int pos_count = 0, neg_count = 0;
            
            for (const auto& clause : clauses) {
                if (clause_satisfied(clause, assignment)) continue;
                
                for (int lit : clause) {
                    if (abs(lit) - 1 == v) {
                        if (lit > 0) pos_count++;
                        else neg_count++;
                    }
                }
            }
            
            int total = pos_count + neg_count;
            if (total > 0) {
                // Golden ratio balance: prefer balanced variables
                double balance = std::abs(pos_count - neg_count) / (double)total;
                double score = total / (1.0 + balance * PHI);
                
                if (score > best_score) {
                    best_score = score;
                    best_var = v;
                }
            }
        }
        
        return best_var;
    }
    
    // Recursive DPLL
    static bool dpll(std::vector<std::vector<int>> clauses,
                    std::vector<int>& assignment,
                    int num_assigned,
                    int num_vars,
                    long long& steps,
                    long long& backtracks) {
        steps++;
        
        // Unit propagation
        if (!unit_propagate(clauses, assignment, num_assigned)) {
            backtracks++;
            return false;
        }
        
        // Check if complete
        if (num_assigned == num_vars) {
            return all_satisfied(clauses, assignment);
        }
        
        // Select variable
        int var = select_variable(clauses, assignment, num_vars);
        if (var == -1) {
            // No variable to assign - should not happen
            return num_assigned == num_vars;
        }
        
        // Try TRUE first
        assignment[var] = 1;
        if (dpll(clauses, assignment, num_assigned + 1, num_vars, steps, backtracks)) {
            return true;
        }
        
        // Try FALSE
        assignment[var] = 0;
        if (dpll(clauses, assignment, num_assigned + 1, num_vars, steps, backtracks)) {
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
        result.golden_score = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<int> assignment(num_vars, -1);
        int num_assigned = 0;
        
        result.satisfiable = dpll(clauses, assignment, num_assigned, 
                                 num_vars, result.steps, result.backtracks);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        result.assignment.resize(num_vars);
        for (int i = 0; i < num_vars; i++) {
            result.assignment[i] = (assignment[i] == 1);
        }
        
        return result;
    }
};

} // namespace GoldenLearningFixed
