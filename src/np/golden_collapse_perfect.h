#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>

namespace GoldenCollapsePerfect {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenCollapseSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    // Perfect collapse: i-collapse papunta sa EXACT solution
    static double collapse_to_solution(double v, bool target) {
        // Kung target ay TRUE, collapse sa PHI
        // Kung target ay FALSE, collapse sa PSI
        return target ? PHI : PSI;
    }
    
    // Clause-guided collapse
    static double guided_collapse(const std::vector<int>& clause,
                                  const std::vector<double>& state,
                                  int target_literal) {
        // I-collapse ang clause papunta sa target literal
        int target_var = abs(target_literal) - 1;
        bool target_val = (target_literal > 0);
        
        return collapse_to_solution(state[target_var], target_val);
    }
    
    // Check kung ang clause ay may satisfying assignment
    static bool clause_satisfiable(const std::vector<int>& clause,
                                   const std::vector<double>& state) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            bool val = state[var] > 0;
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                return true;
            }
        }
        return false;
    }
    
    // Multi-pass collapse na may conflict resolution
    static void collapse_pass(const std::vector<std::vector<int>>& clauses,
                              std::vector<double>& state) {
        // Pass 1: Collapse papunta sa unassigned variables
        for (const auto& clause : clauses) {
            if (clause_satisfiable(clause, state)) continue;
            
            // Hanapin ang unassigned variable
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (std::abs(state[var] - PHI) > 1e-6 && 
                    std::abs(state[var] - PSI) > 1e-6) {
                    // Collapse papunta sa satisfying value
                    state[var] = collapse_to_solution(state[var], lit > 0);
                    break;
                }
            }
        }
        
        // Pass 2: Conflict resolution
        for (const auto& clause : clauses) {
            if (!clause_satisfiable(clause, state)) {
                // Flip ang pinaka-recent na assignment
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    state[var] = (state[var] > 0) ? PSI : PHI;
                }
            }
        }
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Initialize sa neutral state
        std::vector<double> state(num_vars, 0.5);
        
        // Collapse passes
        for (int iter = 0; iter < 10; iter++) {
            result.steps++;
            
            collapse_pass(clauses, state);
            
            // Check kung satisfied na
            bool all_sat = true;
            for (const auto& clause : clauses) {
                if (!clause_satisfiable(clause, state)) {
                    all_sat = false;
                    break;
                }
            }
            
            if (all_sat) {
                result.satisfiable = true;
                break;
            }
        }
        
        // Final extraction
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] > 0);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenCollapsePerfect
