#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>

namespace GoldenExact {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenExactSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

    // Golden algebra operations (exact)
    static double golden_or(double x, double y) {
        return PHI - (PHI - x) * (PHI - y) / (PHI - PSI);
    }
    
    static double golden_not(double x) {
        return (x > 0) ? PSI : PHI;
    }
    
    // Evaluate clause exactly
    static bool clause_satisfied(const std::vector<int>& clause,
                                 const std::vector<bool>& assignment) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            bool val = assignment[var];
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                return true;
            }
        }
        return false;
    }
    
    // Golden ratio guided exact search
    static bool golden_search(const std::vector<std::vector<int>>& clauses,
                             std::vector<bool>& assignment,
                             int var_index,
                             long long& steps) {
        steps++;
        
        // Base case: check kung complete
        if (var_index == assignment.size()) {
            for (const auto& clause : clauses) {
                if (!clause_satisfied(clause, assignment)) {
                    return false;
                }
            }
            return true;
        }
        
        // Golden ratio decision: try TRUE first (φ > 0)
        assignment[var_index] = true;
        if (golden_search(clauses, assignment, var_index + 1, steps)) {
            return true;
        }
        
        // Try FALSE (ψ < 0)
        assignment[var_index] = false;
        if (golden_search(clauses, assignment, var_index + 1, steps)) {
            return true;
        }
        
        return false;
    }
    
    // Golden ratio branch and bound
    static bool golden_branch_bound(const std::vector<std::vector<int>>& clauses,
                                   std::vector<bool>& assignment,
                                   int var_index,
                                   long long& steps) {
        steps++;
        
        // Check kung may conflict na
        for (const auto& clause : clauses) {
            bool has_unassigned = false;
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (var >= var_index) {
                    has_unassigned = true;
                    break;
                }
            }
            
            if (!has_unassigned && !clause_satisfied(clause, assignment)) {
                return false;  // Conflict!
            }
        }
        
        // Base case
        if (var_index == assignment.size()) {
            return true;
        }
        
        // Try TRUE
        assignment[var_index] = true;
        if (golden_branch_bound(clauses, assignment, var_index + 1, steps)) {
            return true;
        }
        
        // Try FALSE
        assignment[var_index] = false;
        if (golden_branch_bound(clauses, assignment, var_index + 1, steps)) {
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
        
        std::vector<bool> assignment(num_vars, false);
        
        // Use branch and bound (mas efficient kaysa plain search)
        result.satisfiable = golden_branch_bound(clauses, assignment, 0, result.steps);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        result.assignment = assignment;
        
        return result;
    }
};

} // namespace GoldenExact
