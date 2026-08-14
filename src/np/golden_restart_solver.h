#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <random>
#include <algorithm>

namespace GoldenRestart {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenRestartSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long restarts;
        double time_ms;
    };

private:
    // Fixed point iteration with golden ratio restart
    static double fixed_point_iterate(double x) {
        if (x >= 0) return std::sqrt(x + 1.0);
        return -1.0 / x;
    }
    
    static double clause_energy(const std::vector<int>& clause,
                                const std::vector<double>& state) {
        bool any_true = false;
        
        for (int lit : clause) {
            int var = abs(lit) - 1;
            double val = state[var];
            bool is_true = (lit > 0 && val > 0) || (lit < 0 && val < 0);
            if (is_true) {
                any_true = true;
                break;
            }
        }
        
        if (any_true) return 0.0;
        
        double energy = 0.0;
        for (int lit : clause) {
            int var = abs(lit) - 1;
            double val = state[var];
            double target = (lit > 0) ? PHI : PSI;
            energy += (val - target) * (val - target);
        }
        return energy;
    }
    
    static double total_energy(const std::vector<std::vector<int>>& clauses,
                               const std::vector<double>& state) {
        double total = 0.0;
        for (const auto& clause : clauses) {
            total += clause_energy(clause, state);
        }
        return total;
    }
    
    // Golden ratio restart: mag-jump sa ibang region
    static void golden_restart(std::vector<double>& state, int restart_count) {
        std::random_device rd;
        std::mt19937 gen(rd() + restart_count);
        
        for (int v = 0; v < state.size(); v++) {
            // Golden ratio guided random restart
            double golden_angle = 2.0 * M_PI * v / PHI + restart_count * M_PI / PHI;
            state[v] = std::sin(golden_angle) * PHI;
        }
    }
    
    // Gradient descent with golden ratio restarts
    static void gradient_descent(const std::vector<std::vector<int>>& clauses,
                                 std::vector<double>& state,
                                 int max_iterations = 100) {
        double epsilon = 1e-6;
        
        for (int iter = 0; iter < max_iterations; iter++) {
            double current_energy = total_energy(clauses, state);
            
            if (current_energy < 1e-10) break;
            
            // Compute gradient
            std::vector<double> gradient(state.size(), 0.0);
            
            for (int v = 0; v < state.size(); v++) {
                std::vector<double> state_plus = state;
                state_plus[v] += epsilon;
                double energy_plus = total_energy(clauses, state_plus);
                
                std::vector<double> state_minus = state;
                state_minus[v] -= epsilon;
                double energy_minus = total_energy(clauses, state_minus);
                
                gradient[v] = (energy_plus - energy_minus) / (2 * epsilon);
            }
            
            // Update with golden ratio step
            double step = 1.0 / PHI;
            for (int v = 0; v < state.size(); v++) {
                state[v] -= gradient[v] * step;
            }
        }
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.restarts = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<double> best_state(num_vars, PHI);
        double best_energy = total_energy(clauses, best_state);
        
        // Multiple restarts with golden ratio
        for (int restart = 0; restart < 20; restart++) {
            result.restarts++;
            
            std::vector<double> state(num_vars, PHI);
            golden_restart(state, restart);
            
            gradient_descent(clauses, state);
            
            double energy = total_energy(clauses, state);
            
            if (energy < best_energy) {
                best_energy = energy;
                best_state = state;
                
                result.steps += 100;  // Approximation
                
                if (energy < 1e-10) {
                    result.satisfiable = true;
                    break;
                }
            }
        }
        
        // Extract assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = best_state[v] > 0;
        }
        
        // Verify
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
        
        result.satisfiable = (satisfied == clauses.size());
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenRestart
