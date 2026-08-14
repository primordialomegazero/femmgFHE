#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <complex>

namespace GoldenCollapse {

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
    // FGG Collapse - ang core operation!
    static double collapse(double v, int depth = 3) {
        double c = v;
        for (int i = 0; i < depth; i++) {
            // φ·ψ = -1, kaya ito ay alternating collapse
            c = c * (i % 2 == 0 ? PSI : PHI);
            // Normalize para hindi mag-overflow
            if (std::abs(c) > 1e10) c = std::copysign(1e10, c);
            if (std::abs(c) < 1e-10) c = 0;
        }
        return c;
    }
    
    // Golden collapse para sa buong formula
    static double formula_collapse(const std::vector<std::vector<int>>& clauses,
                                   std::vector<double>& state) {
        double total = 0.0;
        
        for (const auto& clause : clauses) {
            // I-collapse ang clause papunta sa satisfied state
            double clause_val = 1.0;
            
            for (int lit : clause) {
                int var = abs(lit) - 1;
                double val = state[var];
                
                if (lit < 0) val = -val;  // Negation
                
                // Multiplicative collapse (hindi addition!)
                clause_val *= collapse(val);
            }
            
            // Satisfied kung ang product ay positive (φ^n o ψ^n)
            total += clause_val;
        }
        
        return total;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Initialize sa superposed state (lahat ng possibilities)
        std::vector<double> state(num_vars, 0.5);  // Neutral
        
        // Collapse iterations
        for (int iter = 0; iter < 1000; iter++) {
            result.steps++;
            
            // Collapse ang bawat variable
            for (int v = 0; v < num_vars; v++) {
                // Collapse papunta sa φ o ψ
                state[v] = collapse(state[v]);
                
                // Project sa nearest orbit
                if (std::abs(state[v] - PHI) < std::abs(state[v] - PSI)) {
                    state[v] = PHI;
                } else if (std::abs(state[v] - PSI) < std::abs(state[v] - PHI)) {
                    state[v] = PSI;
                }
            }
            
            // Check kung satisfied na
            double energy = formula_collapse(clauses, state);
            
            if (std::abs(energy) < 1e-6) {
                result.satisfiable = true;
                break;
            }
            
            // I-adjust ang state para sa unsatisfied clauses
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
                    // Collapse ang unsatisfied clause
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        state[var] = collapse(state[var]);
                    }
                }
            }
        }
        
        // Extract assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] > 0);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenCollapse
