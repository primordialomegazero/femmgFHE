#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <complex>
#include <chrono>
#include <random>

namespace SpiralGoldenSAT {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// Spiral Golden Framework: φ·ψ = -1 = 1+1=2
// Ang -1 ay nagbibigay ng complete cycle
// Ang 2 ay nagbibigay ng binary decision
class SpiralGoldenSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
        double spiral_score;
    };

private:
    // Spiral traversal ng solution space
    static std::complex<double> spiral_step(int var, int depth) {
        // Golden spiral: r = φ^(θ/π)
        double theta = 2.0 * M_PI * var / PHI;
        double r = std::pow(PHI, depth / M_PI);
        return std::complex<double>(r * std::cos(theta), r * std::sin(theta));
    }
    
    // Check kung ang complex point ay nasa "satisfying region"
    static bool spiral_measure(const std::complex<double>& z) {
        // φ·ψ = -1 cycle
        return std::real(z) * std::imag(z) > 0;
    }
    
    static double evaluate(const std::vector<std::vector<int>>& clauses,
                          const std::vector<bool>& assignment) {
        int satisfied = 0;
        for (const auto& clause : clauses) {
            bool sat = false;
            for (int lit : clause) {
                int var = abs(lit) - 1;
                bool val = assignment[var];
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    sat = true;
                    break;
                }
            }
            if (sat) satisfied++;
        }
        return (double)satisfied / clauses.size();
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.time_ms = 0;
        result.spiral_score = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Spiral descent: parang gradient descent pero sa spiral coordinates
        std::vector<std::complex<double>> points(num_vars);
        std::vector<bool> best_assignment(num_vars);
        double best_score = 0;
        
        // Initialize sa golden spiral
        for (int v = 0; v < num_vars; v++) {
            points[v] = spiral_step(v, 0);
        }
        
        // Spiral iterations
        for (int depth = 0; depth < num_vars * 2; depth++) {
            result.steps++;
            
            // Measure current points
            std::vector<bool> assignment(num_vars);
            for (int v = 0; v < num_vars; v++) {
                assignment[v] = spiral_measure(points[v]);
            }
            
            // Evaluate
            double score = evaluate(clauses, assignment);
            
            if (score > best_score) {
                best_score = score;
                best_assignment = assignment;
            }
            
            if (score == 1.0) {
                result.satisfiable = true;
                result.assignment = assignment;
                result.spiral_score = score;
                break;
            }
            
            // Spiral update: rotate towards unsatisfied clauses
            for (const auto& clause : clauses) {
                bool sat = false;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    bool val = assignment[var];
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        sat = true;
                        break;
                    }
                }
                
                if (!sat) {
                    // Rotate points towards satisfying assignment
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        double angle = (lit > 0) ? M_PI / PHI : -M_PI / PHI;
                        points[var] *= std::complex<double>(std::cos(angle), std::sin(angle));
                    }
                }
            }
        }
        
        if (!result.satisfiable) {
            result.assignment = best_assignment;
            result.spiral_score = best_score;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace SpiralGoldenSAT
