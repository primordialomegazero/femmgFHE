#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <random>

namespace GoldenSoftV2 {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenSoftSolverV2 {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long propagations;
        double time_ms;
    };

private:
    static int count_unsat(const std::vector<std::vector<int>>& clauses,
                           const std::vector<int>& state) {
        int unsat = 0;
        for (const auto& clause : clauses) {
            bool sat = false;
            for (int lit : clause) {
                int var = abs(lit) - 1;
                bool val = state[var] == 1;
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    sat = true;
                    break;
                }
            }
            if (!sat) unsat++;
        }
        return unsat;
    }

    // Soft propagation na may annealing
    static void soft_propagate_annealing(
        const std::vector<std::vector<int>>& clauses,
        std::vector<double>& probs,
        int num_vars,
        long long& propagation_count,
        double temperature = 1.0) {
        
        // Random initialization na may golden ratio bias
        std::random_device rd;
        std::mt19937 gen(42);
        std::uniform_real_distribution<> dis(-0.2, 0.2);
        
        for (int v = 0; v < num_vars; v++) {
            probs[v] = 0.5 + dis(gen);  // May variation
            probs[v] = std::max(0.1, std::min(0.9, probs[v]));
        }
        
        // Iterative na may annealing
        for (int iter = 0; iter < 100; iter++) {
            std::vector<double> new_probs = probs;
            
            // Update batay sa clauses
            for (const auto& clause : clauses) {
                // Compute kung gaano ka-unsat ang clause
                double prob_unsat = 1.0;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    double prob_true = probs[var];
                    double prob_lit_true = (lit > 0) ? prob_true : (1 - prob_true);
                    prob_unsat *= (1 - prob_lit_true);
                }
                
                // Kung may chance na unsat, i-adjust
                if (prob_unsat > 0.2) {
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        double adjustment = temperature * prob_unsat * 0.05;
                        
                        if (lit > 0) {
                            new_probs[var] += adjustment;
                        } else {
                            new_probs[var] -= adjustment;
                        }
                    }
                }
            }
            
            // Golden ratio projection
            for (int v = 0; v < num_vars; v++) {
                // I-project papunta sa golden ratio kung malapit
                if (new_probs[v] > 0.7) {
                    new_probs[v] = 0.8;
                } else if (new_probs[v] < 0.3) {
                    new_probs[v] = 0.2;
                }
                
                // Clamp
                new_probs[v] = std::max(0.05, std::min(0.95, new_probs[v]));
            }
            
            // Check convergence
            double change = 0.0;
            for (int v = 0; v < num_vars; v++) {
                change += std::abs(new_probs[v] - probs[v]);
            }
            
            probs = new_probs;
            propagation_count++;
            
            // I-update ang temperature (cooling)
            temperature *= 0.95;
            
            if (change < 0.001) break;
        }
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars,
                        int max_attempts = 100) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.propagations = 0;

        auto start = std::chrono::high_resolution_clock::now();

        for (int attempt = 0; attempt < max_attempts && !result.satisfiable; attempt++) {
            result.steps = attempt + 1;

            // Soft propagation with annealing
            std::vector<double> probs(num_vars);
            double temperature = 1.0 / (1.0 + 0.05 * attempt);  // Bumababa sa bawat attempt
            soft_propagate_annealing(clauses, probs, num_vars, result.propagations, temperature);

            // I-convert sa binary assignment
            std::vector<int> state(num_vars);
            for (int v = 0; v < num_vars; v++) {
                state[v] = (probs[v] > 0.5) ? 1 : -1;
            }

            // Check kung satisfied
            int unsat = count_unsat(clauses, state);

            if (unsat == 0) {
                result.satisfiable = true;
                result.assignment.resize(num_vars);
                for (int v = 0; v < num_vars; v++) {
                    result.assignment[v] = (state[v] == 1);
                }
                break;
            }

            // Kung malapit na (less than 10% unsat), local search
            if (unsat < clauses.size() * 0.1) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, num_vars - 1);

                for (int flip = 0; flip < 200; flip++) {
                    int best_var = -1;
                    int best_unsat = unsat + 1;

                    // Random sampling ng 20 variables
                    for (int s = 0; s < std::min(num_vars, 20); s++) {
                        int v = dis(gen);
                        state[v] = 1 - state[v];
                        int new_unsat = count_unsat(clauses, state);
                        state[v] = 1 - state[v];

                        if (new_unsat < best_unsat) {
                            best_unsat = new_unsat;
                            best_var = v;
                        }
                    }

                    if (best_var == -1) break;

                    state[best_var] = 1 - state[best_var];
                    unsat = best_unsat;

                    if (unsat == 0) {
                        result.satisfiable = true;
                        result.assignment.resize(num_vars);
                        for (int v = 0; v < num_vars; v++) {
                            result.assignment[v] = (state[v] == 1);
                        }
                        break;
                    }
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenSoftV2
