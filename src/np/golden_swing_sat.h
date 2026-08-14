#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>

namespace GoldenSwing {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenSwingSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    // Fibonacci swing: ang core operation sa FHE mo!
    static double swing(double v) { 
        return -1.0 / v; 
    }
    
    // Golden ratio projection
    static double project(double v) {
        if (std::abs(v - PHI) < std::abs(v - PSI)) return PHI;
        return PSI;
    }
    
    // Evaluate clause gamit ang golden algebra
    static double eval_clause(const std::vector<int>& clause,
                             const std::vector<double>& state) {
        // OR operation via golden ratio
        double result = PSI;  // Start with FALSE
        
        for (int lit : clause) {
            int var = abs(lit) - 1;
            double val = state[var];
            if (lit < 0) val = swing(val);  // NOT via swing!
            
            // OR via swing
            result = swing(swing(result) * swing(val));
        }
        
        return result;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Initialize sa φ-orbit
        std::vector<double> state(num_vars, PHI);
        
        // Golden swing iterations
        for (int iter = 0; iter < 1000; iter++) {
            result.steps++;
            
            // Check lahat ng clauses
            bool all_sat = true;
            for (const auto& clause : clauses) {
                double val = eval_clause(clause, state);
                // Satisfied kung malapit sa PHI
                if (std::abs(val - PHI) > 1e-6) {
                    all_sat = false;
                    // Swing ang variables sa unsatisfied clause
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        state[var] = swing(state[var]);
                    }
                }
            }
            
            if (all_sat) {
                result.satisfiable = true;
                break;
            }
            
            // Project pabalik sa orbit
            for (int v = 0; v < num_vars; v++) {
                state[v] = project(state[v]);
            }
        }
        
        // Extract assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] == PHI);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenSwing
