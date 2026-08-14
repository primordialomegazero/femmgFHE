#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>

namespace BlackHoleSAT {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class BlackHoleSATSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    // Black hole absorb: i-collapse sa |v| via φ·ψ = -1
    static double absorb(double v, int horizon = 3) {
        double result = v;
        for (int d = 0; d < horizon; d++) {
            result = std::abs(result * PHI * PSI);
        }
        return result;
    }
    
    // Black hole entropy: S = φ²·A/4
    static double entropy(double area) {
        return PHI * PHI * area / 4.0;
    }
    
    // Black hole temperature: T = 1/(8πMφ²)
    static double temperature(double mass) {
        return 1.0 / (8.0 * M_PI * mass * PHI * PHI);
    }
    
    // I-collapse ang buong formula sa black hole
    static void collapse_formula(const std::vector<std::vector<int>>& clauses,
                                 std::vector<double>& state) {
        // Para sa bawat clause, i-absorb ang "noise"
        for (const auto& clause : clauses) {
            bool satisfied = false;
            
            for (int lit : clause) {
                int var = abs(lit) - 1;
                double val = state[var];
                
                if (lit < 0) val = -val;
                
                if (val > 0) {
                    satisfied = true;
                    break;
                }
            }
            
            if (!satisfied) {
                // Unsatisfied clause = noise
                // I-absorb sa black hole
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    state[var] = absorb(state[var]);
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
        
        // Black hole collapse iterations
        for (int iter = 0; iter < 100; iter++) {
            result.steps++;
            
            collapse_formula(clauses, state);
            
            // Check kung satisfied na
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

} // namespace BlackHoleSAT
