#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>

namespace GoldenDPLL {

constexpr double PHI = 1.6180339887498948482;

class GoldenDPLLSolver {
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
    // Golden ratio weighted variable selection
    static int select_variable(const std::vector<std::vector<int>>& clauses,
                              const std::vector<int>& assignment, int num_vars) {
        int best_var = -1;
        double best_score = -1e18;

        for (int v = 0; v < num_vars; v++) {
            if (assignment[v] == -1) {
                int pos_count = 0;
                int neg_count = 0;
                int total_appearances = 0;

                // Count appearances in unsatisfied clauses
                for (const auto& clause : clauses) {
                    bool clause_satisfied = false;
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        if (var < assignment.size() && assignment[var] != -1) {
                            bool val = assignment[var] == 1;
                            if ((lit > 0 && val) || (lit < 0 && !val)) {
                                clause_satisfied = true;
                                break;
                            }
                        }
                    }
                    
                    if (!clause_satisfied) {
                        for (int lit : clause) {
                            if (abs(lit) - 1 == v) {
                                total_appearances++;
                                if (lit > 0) pos_count++;
                                else neg_count++;
                            }
                        }
                    }
                }
                
                // Golden ratio balanced scoring
                double balance = std::abs(pos_count - neg_count) * PHI;
                double activity = total_appearances * PHI;
                double golden_score = activity / (balance + 1.0);
                
                if (golden_score > best_score) {
                    best_score = golden_score;
                    best_var = v;
                }
            }
        }
        
        return best_var;
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
        
        // Check if complete
        if (num_assigned == num_vars) {
            for (const auto& clause : clauses) {
                bool clause_sat = false;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    bool val = assignment[var] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        clause_sat = true;
                        break;
                    }
                }
                if (!clause_sat) {
                    backtracks++;
                    return false;
                }
            }
            return true;
        }
        
        // Golden ratio variable selection
        int best_var = select_variable(clauses, assignment, num_vars);
        
        if (best_var == -1) {
            return num_assigned == num_vars;
        }
        
        // Try TRUE first (golden ratio suggests balance)
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
        result.golden_score = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<int> assignment(num_vars, -1);
        int num_assigned = 0;
        
        result.satisfiable = dpll_recursive(clauses, assignment, num_assigned, 
                                           num_vars, result.steps, result.backtracks);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        result.assignment.resize(num_vars);
        for (int i = 0; i < num_vars; i++) {
            result.assignment[i] = (assignment[i] == 1);
        }
        
        // Calculate golden score
        int satisfied = 0;
        for (const auto& clause : clauses) {
            bool clause_sat = false;
            for (int lit : clause) {
                int var = abs(lit) - 1;
                bool val = result.assignment[var];
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    clause_sat = true;
                    break;
                }
            }
            if (clause_sat) satisfied++;
        }
        result.golden_score = (double)satisfied / clauses.size() * PHI;
        
        return result;
    }
};

} // namespace GoldenDPLL
