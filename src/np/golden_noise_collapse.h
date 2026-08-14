#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>

namespace GoldenNoiseCollapse {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenNoiseSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    // FGG collapse para sa noise absorption
    static double fgg_collapse(double v, int level = 3) {
        double c = v;
        for (int i = 0; i < level; i++) {
            c = c * (i % 2 == 0 ? PSI : PHI);
            // Hindi absolute - pinapanatili ang sign!
        }
        return c;
    }
    
    // I-collapse ang noise (unsatisfied clauses) papunta sa assignment
    static void collapse_noise(const std::vector<std::vector<int>>& clauses,
                               std::vector<double>& state) {
        for (const auto& clause : clauses) {
            bool satisfied = false;
            
            for (int lit : clause) {
                int var = abs(lit) - 1;
                bool val = state[var] > 0;
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    satisfied = true;
                    break;
                }
            }
            
            if (!satisfied) {
                // Unsatisfied clause = noise
                // I-collapse ang variables sa clause papunta sa satisfying assignment
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    double target = (lit > 0) ? PHI : PSI;
                    state[var] = fgg_collapse(target);
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
        
        // Initialize sa neutral state (hindi φ o ψ, kundi 0)
        std::vector<double> state(num_vars, 0.0);
        
        // Collapse iterations
        for (int iter = 0; iter < 100; iter++) {
            result.steps++;
            
            collapse_noise(clauses, state);
            
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
            result.assignment[v] = state[v] > 0;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenNoiseCollapse
