#pragma once
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>
#include <random>
#include <algorithm>

namespace GoldenMultiplicativeEnhanced {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenMultiplicativeSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        double golden_score;
        long long steps;
    };

private:
    static double evaluate(const std::vector<std::vector<int>>& clauses, 
                          const std::vector<bool>& assignment) {
        int satisfied = 0;
        for (const auto& clause : clauses) {
            bool clause_sat = false;
            for (int lit : clause) {
                int var = abs(lit) - 1;
                bool val = assignment[var];
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    clause_sat = true;
                    break;
                }
            }
            if (clause_sat) satisfied++;
        }
        // Golden ratio weighted score
        return (double)satisfied * PHI + (double)(clauses.size() - satisfied) * PSI;
    }
    
    static void random_restart(std::vector<bool>& assignment, std::mt19937& gen) {
        std::uniform_int_distribution<> dis(0, 1);
        for (size_t i = 0; i < assignment.size(); i++) {
            assignment[i] = dis(gen) == 1;
        }
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars, 
                       int max_restarts = 20, int max_iterations = 1000) {
        Result result;
        result.satisfiable = false;
        result.golden_score = 0.0;
        result.steps = 0;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> prob(0.0, 1.0);
        
        std::vector<bool> best_assignment(num_vars, true);
        double best_score = -1e18;
        
        for (int restart = 0; restart < max_restarts; restart++) {
            std::vector<bool> assignment(num_vars, true);
            if (restart > 0) {
                random_restart(assignment, gen);
            }
            
            double temperature = 1.0;
            int iterations = std::min(max_iterations, num_vars * 20);
            
            for (int iter = 0; iter < iterations; iter++) {
                result.steps++;
                
                // Check if all clauses satisfied
                double current_score = evaluate(clauses, assignment);
                int satisfied_count = 0;
                for (const auto& clause : clauses) {
                    bool clause_sat = false;
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        bool val = assignment[var];
                        if ((lit > 0 && val) || (lit < 0 && !val)) {
                            clause_sat = true;
                            break;
                        }
                    }
                    if (clause_sat) satisfied_count++;
                }
                
                if (satisfied_count == clauses.size()) {
                    result.satisfiable = true;
                    result.assignment = assignment;
                    result.golden_score = current_score;
                    return result;
                }
                
                // Simulated annealing with golden ratio
                temperature *= 0.95;
                
                // Try flipping variables with best improvement
                int best_var = -1;
                double best_improvement = 0;
                
                for (int v = 0; v < num_vars; v++) {
                    assignment[v] = !assignment[v];
                    double new_score = evaluate(clauses, assignment);
                    double improvement = new_score - current_score;
                    assignment[v] = !assignment[v];
                    
                    // Golden ratio acceptance criterion
                    if (improvement > best_improvement || 
                        (improvement > 0 && prob(gen) < 1.0/PHI)) {
                        best_improvement = improvement;
                        best_var = v;
                    }
                }
                
                if (best_var != -1 && (best_improvement > 0 || 
                    prob(gen) < std::exp(best_improvement / (temperature * PHI)))) {
                    assignment[best_var] = !assignment[best_var];
                    current_score += best_improvement;
                    
                    if (current_score > best_score) {
                        best_score = current_score;
                        best_assignment = assignment;
                    }
                }
            }
        }
        
        // Use best assignment found
        result.assignment = best_assignment;
        result.golden_score = best_score;
        
        // Final check
        int satisfied_count = 0;
        for (const auto& clause : clauses) {
            bool clause_sat = false;
            for (int lit : clause) {
                int var = abs(lit) - 1;
                bool val = best_assignment[var];
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    clause_sat = true;
                    break;
                }
            }
            if (clause_sat) satisfied_count++;
        }
        
        result.satisfiable = (satisfied_count == clauses.size());
        return result;
    }
};

} // namespace GoldenMultiplicativeEnhanced
