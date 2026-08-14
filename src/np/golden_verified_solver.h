#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>

namespace GoldenVerified {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenVerifiedSolver {
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
        double activity;
        
        Clause(const std::vector<int>& lits) : literals(lits), activity(0) {}
    };
    
    // FULL verification na walang shortcut
    static bool verify_all_clauses(const std::vector<Clause>& clauses,
                                   const std::vector<int>& assignment) {
        for (const auto& clause : clauses) {
            bool sat = false;
            for (int lit : clause.literals) {
                int var = abs(lit) - 1;
                if (var < assignment.size() && assignment[var] != -1) {
                    bool val = assignment[var] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        sat = true;
                        break;
                    }
                }
            }
            if (!sat) return false;
        }
        return true;
    }
    
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
    
    static bool unit_propagate(std::vector<Clause>& clauses,
                              std::vector<int>& assignment,
                              int& num_assigned) {
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (auto& clause : clauses) {
                if (clause_satisfied(clause, assignment)) continue;
                
                int unassigned = 0;
                int last_lit = 0;
                
                for (int lit : clause.literals) {
                    int var = abs(lit) - 1;
                    if (var < assignment.size() && assignment[var] == -1) {
                        unassigned++;
                        last_lit = lit;
                    }
                }
                
                if (unassigned == 0) {
                    return false;  // Conflict
                } else if (unassigned == 1) {
                    int var = abs(last_lit) - 1;
                    assignment[var] = (last_lit > 0) ? 1 : 0;
                    num_assigned++;
                    changed = true;
                }
            }
        }
        return true;
    }
    
    static int select_variable(const std::vector<Clause>& clauses,
                              const std::vector<int>& assignment,
                              int num_vars) {
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
                double balance = std::abs(pos_count - neg_count) / (double)total;
                double golden_balance = std::pow(PHI, -balance);
                double score = total * golden_balance;
                
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
                              long long& conflicts) {
        steps++;
        
        // Unit propagation
        if (!unit_propagate(clauses, assignment, num_assigned)) {
            conflicts++;
            backtracks++;
            return false;
        }
        
        // Check completion - FULL VERIFICATION
        if (num_assigned == num_vars) {
            return verify_all_clauses(clauses, assignment);
        }
        
        // Select variable
        int var = select_variable(clauses, assignment, num_vars);
        
        if (var == -1) {
            // No variable to assign, check if all satisfied
            return verify_all_clauses(clauses, assignment);
        }
        
        // Try TRUE
        assignment[var] = 1;
        if (dpll_recursive(clauses, assignment, num_assigned + 1, num_vars,
                          steps, backtracks, conflicts)) {
            return true;
        }
        
        // Try FALSE
        assignment[var] = 0;
        if (dpll_recursive(clauses, assignment, num_assigned + 1, num_vars,
                          steps, backtracks, conflicts)) {
            return true;
        }
        
        // Backtrack
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
        
        std::vector<Clause> clauses;
        for (const auto& c : clauses_input) {
            clauses.emplace_back(c);
        }
        
        std::vector<int> assignment(num_vars, -1);
        int num_assigned = 0;
        
        result.satisfiable = dpll_recursive(clauses, assignment, num_assigned,
                                           num_vars, result.steps, result.backtracks,
                                           result.conflicts);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        result.assignment.resize(num_vars);
        for (int i = 0; i < num_vars; i++) {
            result.assignment[i] = (assignment[i] == 1);
        }
        
        return result;
    }
};

} // namespace GoldenVerified
