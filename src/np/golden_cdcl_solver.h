#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <set>

namespace GoldenCDCL {

constexpr double PHI = 1.6180339887498948482;

class GoldenCDCLSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long backtracks;
        long long conflicts;
        double time_ms;
    };

private:
    struct Clause {
        std::vector<int> literals;
        bool is_learned;
        double activity;
        
        Clause(const std::vector<int>& lits, bool learned = false) 
            : literals(lits), is_learned(learned), activity(0) {}
    };
    
    static bool clause_satisfied(const Clause& clause, const std::vector<int>& assignment) {
        for (int lit : clause.literals) {
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
    
    static int get_unassigned_count(const Clause& clause, const std::vector<int>& assignment) {
        int count = 0;
        for (int lit : clause.literals) {
            int var = abs(lit) - 1;
            if (var < assignment.size() && assignment[var] == -1) {
                count++;
            }
        }
        return count;
    }
    
    static int get_last_unassigned(const Clause& clause, const std::vector<int>& assignment) {
        for (int lit : clause.literals) {
            int var = abs(lit) - 1;
            if (var < assignment.size() && assignment[var] == -1) {
                return lit;
            }
        }
        return 0;
    }
    
    static bool unit_propagate(std::vector<Clause>& clauses,
                              std::vector<int>& assignment,
                              int& num_assigned,
                              Clause& conflict_clause) {
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (auto& clause : clauses) {
                if (clause_satisfied(clause, assignment)) continue;
                
                int unassigned = get_unassigned_count(clause, assignment);
                
                if (unassigned == 0) {
                    conflict_clause = clause;
                    return false;
                } else if (unassigned == 1) {
                    int lit = get_last_unassigned(clause, assignment);
                    int var = abs(lit) - 1;
                    assignment[var] = (lit > 0) ? 1 : 0;
                    num_assigned++;
                    changed = true;
                }
            }
        }
        return true;
    }
    
    static int select_variable(const std::vector<Clause>& clauses,
                              const std::vector<int>& assignment,
                              int num_vars,
                              const std::vector<double>& var_activity) {
        int best_var = -1;
        double best_score = -1e18;
        
        for (int v = 0; v < num_vars; v++) {
            if (assignment[v] != -1) continue;
            
            int pos_count = 0, neg_count = 0;
            
            for (const auto& clause : clauses) {
                if (clause_satisfied(clause, assignment)) continue;
                
                for (int lit : clause.literals) {
                    if (abs(lit) - 1 == v) {
                        if (lit > 0) pos_count++;
                        else neg_count++;
                    }
                }
            }
            
            int total = pos_count + neg_count;
            if (total > 0) {
                // Combine golden ratio balance with VSIDS-like activity
                double balance = std::abs(pos_count - neg_count) / (double)total;
                double golden_balance = 1.0 / (1.0 + balance * PHI);
                double score = total * golden_balance * (1.0 + var_activity[v]);
                
                if (score > best_score) {
                    best_score = score;
                    best_var = v;
                }
            }
        }
        
        return best_var;
    }
    
    static bool dpll_recursive(std::vector<Clause>& clauses,
                              std::vector<int>& assignment,
                              int num_assigned,
                              int num_vars,
                              long long& steps,
                              long long& backtracks,
                              long long& conflicts,
                              std::vector<double>& var_activity,
                              int depth) {
        steps++;
        
        // Unit propagation
        Clause conflict_clause({});
        if (!unit_propagate(clauses, assignment, num_assigned, conflict_clause)) {
            conflicts++;
            
            // Conflict analysis: bump activity of variables in conflict
            for (int lit : conflict_clause.literals) {
                int var = abs(lit) - 1;
                if (var < var_activity.size()) {
                    var_activity[var] *= 1.1;  // Increase activity
                }
            }
            
            backtracks++;
            return false;
        }
        
        // Check completion
        if (num_assigned == num_vars) {
            return true;
        }
        
        // Select variable
        int var = select_variable(clauses, assignment, num_vars, var_activity);
        
        if (var == -1) {
            return num_assigned == num_vars;
        }
        
        // Count polarity preference
        int pos_count = 0, neg_count = 0;
        for (const auto& clause : clauses) {
            for (int lit : clause.literals) {
                if (abs(lit) - 1 == var) {
                    if (lit > 0) pos_count++;
                    else neg_count++;
                }
            }
        }
        
        // Try preferred polarity first
        bool try_true = (pos_count >= neg_count);
        
        // Branch
        std::vector<int> assignment_copy = assignment;
        int num_assigned_copy = num_assigned;
        
        assignment[var] = try_true ? 1 : 0;
        if (dpll_recursive(clauses, assignment, num_assigned + 1, num_vars,
                          steps, backtracks, conflicts, var_activity, depth + 1)) {
            return true;
        }
        
        assignment = assignment_copy;
        assignment[var] = try_true ? 0 : 1;
        if (dpll_recursive(clauses, assignment, num_assigned + 1, num_vars,
                          steps, backtracks, conflicts, var_activity, depth + 1)) {
            return true;
        }
        
        assignment[var] = -1;
        backtracks++;
        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses_input, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.backtracks = 0;
        result.conflicts = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Convert to Clause objects
        std::vector<Clause> clauses;
        for (const auto& c : clauses_input) {
            clauses.emplace_back(c, false);
        }
        
        std::vector<int> assignment(num_vars, -1);
        std::vector<double> var_activity(num_vars, 0.0);
        int num_assigned = 0;
        
        result.satisfiable = dpll_recursive(clauses, assignment, num_assigned,
                                           num_vars, result.steps, result.backtracks,
                                           result.conflicts, var_activity, 0);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        result.assignment.resize(num_vars);
        for (int i = 0; i < num_vars; i++) {
            result.assignment[i] = (assignment[i] == 1);
        }
        
        return result;
    }
};

} // namespace GoldenCDCL
