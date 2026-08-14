#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>

namespace GoldenAlgebra {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenAlgebraSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        double energy;
        int iterations;
        double time_ms;
    };

    // Golden ratio truth values
    // TRUE = φ, FALSE = ψ
    static double to_golden(bool val) {
        return val ? PHI : PSI;
    }
    
    static bool from_golden(double val) {
        return val > 0;  // φ > 0, ψ < 0
    }
    
    // Golden OR operation
    // x OR y = φ - (φ-x)(φ-y)/(φ-ψ)
    static double golden_or(double x, double y) {
        return PHI - (PHI - x) * (PHI - y) / (PHI - PSI);
    }
    
    // Golden NOT operation
    static double golden_not(double x) {
        return (x == PHI) ? PSI : PHI;
    }
    
    // Golden AND operation (via De Morgan: x AND y = NOT(NOT x OR NOT y))
    static double golden_and(double x, double y) {
        return golden_not(golden_or(golden_not(x), golden_not(y)));
    }
    
    // Evaluate clause using golden algebra
    static double evaluate_clause(const std::vector<int>& clause,
                                  const std::vector<double>& state) {
        double result = 0.0;
        
        for (int lit : clause) {
            int var = abs(lit) - 1;
            double literal_val = state[var];
            
            if (lit < 0) {
                literal_val = golden_not(literal_val);
            }
            
            if (result == 0.0) {
                result = literal_val;
            } else {
                result = golden_or(result, literal_val);
            }
        }
        
        return result;
    }
    
    // Energy = deviation from φ (satisfied state)
    static double compute_energy(const std::vector<std::vector<int>>& clauses,
                                  const std::vector<double>& state) {
        double energy = 0.0;
        
        for (const auto& clause : clauses) {
            double result = evaluate_clause(clause, state);
            // Satisfied kung malapit sa PHI
            energy += std::pow(result - PHI, 2);
        }
        
        return energy;
    }

    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.energy = 1e18;
        result.iterations = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Initialize sa φ-orbit (lahat TRUE muna)
        std::vector<double> state(num_vars, PHI);
        std::vector<double> best_state = state;
        double best_energy = compute_energy(clauses, state);
        
        // Golden relaxation - gradient descent sa golden space
        for (int iter = 0; iter < 10000; iter++) {
            result.iterations++;
            
            double current_energy = compute_energy(clauses, state);
            
            if (current_energy < best_energy) {
                best_energy = current_energy;
                best_state = state;
            }
            
            // Check convergence
            if (current_energy < 1e-10) {
                result.satisfiable = true;
                result.energy = current_energy;
                break;
            }
            
            // Compute gradient numerically
            std::vector<double> gradient(num_vars, 0.0);
            double epsilon = 0.01;
            
            for (int v = 0; v < num_vars; v++) {
                std::vector<double> state_plus = state;
                state_plus[v] += epsilon;
                gradient[v] = (compute_energy(clauses, state_plus) - current_energy) / epsilon;
            }
            
            // Update state
            for (int v = 0; v < num_vars; v++) {
                state[v] -= gradient[v] * 0.1;
            }
        }
        
        // Extract best assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = from_golden(best_state[v]);
        }
        
        result.energy = best_energy;
        result.satisfiable = (best_energy < 1e-10);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenAlgebra
