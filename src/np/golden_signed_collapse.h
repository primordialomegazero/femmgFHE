#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>

namespace GoldenSignedCollapse {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenSignedSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    // Signed collapse: i-collapse sa φ o ψ (hindi |v|)
    static double signed_collapse(double v, int depth = 3) {
        double result = v;
        for (int d = 0; d < depth; d++) {
            // φ·ψ = -1, pero pinapanatili ang sign
            result = result * (d % 2 == 0 ? PSI : PHI);
        }
        // Project sa nearest golden ratio value
        if (result > 0) return PHI;
        if (result < 0) return PSI;
        return 0;  // Neutral
    }
    
    // Collapse ang formula na may sign preservation
    static void collapse_formula(const std::vector<std::vector<int>>& clauses,
                                 std::vector<double>& state) {
        std::vector<double> new_state = state;
        
        for (const auto& clause : clauses) {
            bool satisfied = false;
            
            for (int lit : clause) {
                int var = abs(lit) - 1;
                double val = state[var];
                bool is_true = (val > 0);
                
                if ((lit > 0 && is_true) || (lit < 0 && !is_true)) {
                    satisfied = true;
                    break;
                }
            }
            
            if (!satisfied) {
                // Unsatisfied clause - collapse papunta sa satisfying assignment
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    // I-collapse papunta sa value na magse-satisfy sa literal
                    double target = (lit > 0) ? PHI : PSI;
                    new_state[var] = signed_collapse(target);
                }
            }
        }
        
        state = new_state;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Initialize sa neutral
        std::vector<double> state(num_vars, 0.0);
        
        // Collapse iterations
        for (int iter = 0; iter < 100; iter++) {
            result.steps++;
            
            collapse_formula(clauses, state);
            
            // Check satisfiability
            bool all_sat = true;
            for (const auto& clause : clauses) {
                bool sat = false;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    bool val = state[var] > 0;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        sat = true;
                        break;
                    }
                }
                if (!sat) {
                    all_sat = false;
                    break;
                }
            }
            
            if (all_sat) {
                result.satisfiable = true;
                break;
            }
        }
        
        // Extract
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] > 0);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenSignedCollapse
