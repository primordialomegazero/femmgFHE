#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <complex>

namespace GoldenProjection {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenProjectionSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        double energy;
        int iterations;
        double time_ms;
    };

private:
    // Project value sa φ-orbit (PHI para sa 1, PSI para sa 0)
    static double project_to_orbit(double value) {
        // Fibonacci projection: i-round sa nearest golden ratio state
        double dist_to_phi = std::abs(value - PHI);
        double dist_to_psi = std::abs(value - PSI);
        
        return (dist_to_phi < dist_to_psi) ? PHI : PSI;
    }
    
    // Energy function na nag-eencode ng clauses bilang golden constraints
    static double compute_energy(const std::vector<std::vector<int>>& clauses,
                                  const std::vector<double>& state) {
        double energy = 0.0;
        
        for (const auto& clause : clauses) {
            // Fibonacci constraint: clause dapat nasa φ-orbit
            double clause_value = 0.0;
            
            for (int lit : clause) {
                int var = abs(lit) - 1;
                double contribution = state[var];
                
                if (lit < 0) {
                    // Negation: flip sa orbit
                    contribution = (contribution == PHI) ? PSI : PHI;
                }
                
                // OR operation bilang golden ratio addition
                clause_value += contribution;
            }
            
            // Satisfied kung ang clause value ay nasa φ-orbit
            // φ + φ = 2φ = φ^2 (golden identity!)
            // φ + ψ = 1 (complete cycle!)
            // ψ + ψ = 2ψ = -φ^2
            double nearest_phi_power = std::pow(PHI, std::round(std::log(std::abs(clause_value)) / std::log(PHI)));
            double deviation = clause_value - nearest_phi_power;
            energy += deviation * deviation;
        }
        
        return energy;
    }
    
    // Golden ratio convergence check
    static bool is_converged(const std::vector<double>& state) {
        for (double value : state) {
            if (value != PHI && value != PSI) {
                return false;
            }
        }
        return true;
    }
    
    // Extract boolean solution mula sa φ-orbit state
    static std::vector<bool> extract_solution(const std::vector<double>& state) {
        std::vector<bool> solution(state.size());
        for (int i = 0; i < state.size(); i++) {
            solution[i] = (state[i] == PHI);
        }
        return solution;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.energy = 1e18;
        result.iterations = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Initialize sa φ-orbit
        std::vector<double> state(num_vars, PHI);
        
        // Golden projection dynamics
        for (int iter = 0; iter < 1000; iter++) {
            result.iterations++;
            
            // Compute current energy
            double energy = compute_energy(clauses, state);
            
            // Update best energy
            if (energy < result.energy) {
                result.energy = energy;
            }
            
            // Check convergence
            if (energy < 1e-10) {
                result.satisfiable = true;
                result.assignment = extract_solution(state);
                break;
            }
            
            // Golden ratio relaxation
            for (int v = 0; v < num_vars; v++) {
                // Gradual projection (hindi hard assignment)
                double current = state[v];
                double target = project_to_orbit(current);
                state[v] = current + (target - current) * (1.0 / PHI);
            }
        }
        
        // Final projection
        for (int v = 0; v < num_vars; v++) {
            state[v] = project_to_orbit(state[v]);
        }
        
        result.assignment = extract_solution(state);
        
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
        result.energy = compute_energy(clauses, state);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenProjection
