#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <set>

namespace GoldenLearning {

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
    // Golden ratio based variable ordering
    static int select_variable(const std::vector<std::vector<int>>& clauses,
                              const std::vector<int>& assignment, int num_vars) {
        int best_var = -1;
        double best_score = -1e18;
        
        for (int v = 0; v < num_vars; v++) {
            if (assignment[v] == -1) {
                int pos_occ = 0, neg_occ = 0;
                
                // Count in unsatisfied clauses
                for (const auto& clause : clauses) {
                    bool sat = false;
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        if (assignment[var] != -1) {
                            bool val = assignment[var] == 1;
                            if ((lit > 0 && val) || (lit < 0 && !val)) {
                                sat = true;
                                break;
                            }
                        }
                    }
                    if (!sat) {
                        for (int lit : clause) {
                            if (abs(lit) - 1 == v) {
                                if (lit > 0) pos_occ++;
                                else neg_occ++;
                            }
                        }
                    }
                }
                
                // Golden ratio balance scoring
                int total = pos_occ + neg_occ;
                if (total > 0) {
                    double balance = abs(pos_occ - neg_occ) / (double)total;
                    double golden_balance = 1.0 / (1.0 + balance * PHI);
                    double score = total * golden_balance;
                    
                    if (score > best_score) {
                        best_score = score;
                        best_var = v;
                    }
                }
            }
        }
        return best_var;
    }
    
    // Unit propagation with conflict detection
    static bool propagate(std::vector<std::vector<int>>& clauses, 
                         std::vector<int>& assignment, int& num_assigned,
                         std::vector<int>& conflict_clause) {
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (auto& clause : clauses) {
                int unassigned = 0;
                int last_lit = 0;
                bool sat = false;
                
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (assignment[var] != -1) {
                        bool val = assignment[var] == 1;
                        if ((lit > 0 && val) || (lit < 0 && !val)) {
                            sat = true;
                            break;
                        }
                    } else {
                        unassigned++;
                        last_lit = lit;
                    }
                }
                
                if (!sat) {
                    if (unassigned == 0) {
                        // Conflict
                        conflict_clause = clause;
                        return false;
                    } else if (unassigned == 1) {
                        // Unit propagation
                        int var = abs(last_lit) - 1;
                        assignment[var] = (last_lit > 0) ? 1 : 0;
                        num_assigned++;
                        changed = true;
                    }
                }
            }
        }
        return true;
    }
    
    static bool dpll_recursive(std::vector<std::vector<int>> clauses, 
                              std::vector<int>& assignment, 
                              int num_assigned, int num_vars,
                              long long& steps, long long& backtracks,
                              int depth = 0) {
        steps++;
        
        // Unit propagation
        std::vector<int> conflict_clause;
        if (!propagate(clauses, assignment, num_assigned, conflict_clause)) {
            backtracks++;
            return false;
        }
        
        // Check completion
        if (num_assigned == num_vars) {
            return true;
        }
        
        // Golden ratio variable selection
        int var = select_variable(clauses, assignment, num_vars);
        
        if (var == -1) {
            return num_assigned == num_vars;
        }
        
        // Value ordering using golden ratio
        // Prefer the polarity that appears more in clauses
        int pos_count = 0, neg_count = 0;
        for (const auto& clause : clauses) {
            for (int lit : clause) {
                if (abs(lit) - 1 == var) {
                    if (lit > 0) pos_count++;
                    else neg_count++;
                }
            }
        }
        
        // Try more frequent polarity first
        bool try_true_first = (pos_count >= neg_count);
        
        if (try_true_first) {
            assignment[var] = 1;
            if (dpll_recursive(clauses, assignment, num_assigned + 1, num_vars, 
                              steps, backtracks, depth + 1)) {
                return true;
            }
            
            assignment[var] = 0;
            if (dpll_recursive(clauses, assignment, num_assigned + 1, num_vars, 
                              steps, backtracks, depth + 1)) {
                return true;
            }
        } else {
            assignment[var] = 0;
            if (dpll_recursive(clauses, assignment, num_assigned + 1, num_vars, 
                              steps, backtracks, depth + 1)) {
                return true;
            }
            
            assignment[var] = 1;
            if (dpll_recursive(clauses, assignment, num_assigned + 1, num_vars, 
                              steps, backtracks, depth + 1)) {
                return true;
            }
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
        
        result.satisfiable = dpll_recursive(clauses, assignment, num_assigned, 
                                           num_vars, result.steps, result.backtracks);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        result.assignment.resize(num_vars);
        for (int i = 0; i < num_vars; i++) {
            result.assignment[i] = (assignment[i] == 1);
        }
        
        // Golden score
        int satisfied = 0;
        for (const auto& clause : clauses) {
            bool sat = false;
            for (int lit : clause) {
                int var = abs(lit) - 1;
                bool val = result.assignment[var];
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    sat = true;
                    break;
                }
            }
            if (sat) satisfied++;
        }
        result.golden_score = (double)satisfied / clauses.size() * PHI;
        
        return result;
    }
};

} // namespace GoldenLearning
