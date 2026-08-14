#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>

namespace GoldenFixedPoint {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenFixedPointSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long iterations;
        double convergence_rate;
        double time_ms;
    };

private:
    // Fixed point equation: x² = x + 1
    // Ang roots ay φ at ψ
    // Kung ang variable value ay nagco-converge sa φ → TRUE
    // Kung nagco-converge sa ψ → FALSE
    static double fixed_point_iterate(double x) {
        // x_{n+1} = sqrt(x_n + 1) → converges to φ
        if (x >= 0) return std::sqrt(x + 1.0);
        // x_{n+1} = -1/(x_n) → converges to ψ
        return -1.0 / x;
    }

    // Check convergence
    static bool converged(double x, double target, double eps = 1e-6) {
        return std::abs(x - target) < eps;
    }

    // Transform clause into "energy" na mina-minimize
    static double clause_energy(const std::vector<int>& clause,
                                const std::vector<double>& state) {
        double energy = 0.0;
        bool any_true = false;

        for (int lit : clause) {
            int var = abs(lit) - 1;
            double val = state[var];

            // Kung literal ay positive, true kung val > 0
            // Kung negative, true kung val < 0
            bool is_true = (lit > 0 && val > 0) || (lit < 0 && val < 0);

            if (is_true) {
                any_true = true;
                break;
            }
        }

        // Kung walang true literal, may energy penalty
        if (!any_true) {
            // Energy = distance from satisfaction
            for (int lit : clause) {
                int var = abs(lit) - 1;
                double val = state[var];
                double target = (lit > 0) ? PHI : PSI;
                energy += (val - target) * (val - target);
            }
        }

        return energy;
    }

    // Total energy ng buong formula
    static double total_energy(const std::vector<std::vector<int>>& clauses,
                               const std::vector<double>& state) {
        double total = 0.0;
        for (const auto& clause : clauses) {
            total += clause_energy(clause, state);
        }
        return total;
    }

    // Gradient ng energy (para mag-descent papunta sa minimum)
    static std::vector<double> gradient(const std::vector<std::vector<int>>& clauses,
                                        const std::vector<double>& state,
                                        int num_vars) {
        std::vector<double> grad(num_vars, 0.0);
        double eps = 1e-6;

        for (int v = 0; v < num_vars; v++) {
            std::vector<double> state_plus = state;
            std::vector<double> state_minus = state;
            state_plus[v] += eps;
            state_minus[v] -= eps;

            double e_plus = total_energy(clauses, state_plus);
            double e_minus = total_energy(clauses, state_minus);
            grad[v] = (e_plus - e_minus) / (2 * eps);
        }

        return grad;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars,
                        int max_iterations = 1000) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.iterations = 0;
        result.convergence_rate = 0.0;

        auto start = std::chrono::high_resolution_clock::now();

        // Initialize state randomly pero may golden influence
        std::vector<double> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            // Start malapit sa golden ratio
            state[v] = (v % 2 == 0) ? PHI : PSI;
        }

        // Iterative fixed point + gradient descent
        std::vector<double> prev_state = state;
        double prev_energy = total_energy(clauses, state);

        for (int iter = 0; iter < max_iterations; iter++) {
            result.iterations = iter + 1;
            result.steps++;

            // Compute gradient
            std::vector<double> grad = gradient(clauses, state, num_vars);

            // Update state: gradient descent + fixed point iteration
            double learning_rate = 0.1 / (1.0 + 0.01 * iter); // Decaying

            for (int v = 0; v < num_vars; v++) {
                state[v] -= learning_rate * grad[v];

                // Fixed point push papunta sa φ o ψ
                if (state[v] > 0) {
                    state[v] = fixed_point_iterate(state[v]);
                } else {
                    state[v] = fixed_point_iterate(state[v]);
                }

                // Clamp para hindi mag-explode
                if (state[v] > 10.0) state[v] = PHI;
                if (state[v] < -10.0) state[v] = PSI;
                if (std::abs(state[v]) < 1e-10) state[v] = (v % 2 == 0) ? PHI : PSI;
            }

            // Check energy
            double energy = total_energy(clauses, state);

            // Convergence check
            double change = 0.0;
            for (int v = 0; v < num_vars; v++) {
                change += std::abs(state[v] - prev_state[v]);
            }
            change /= num_vars;

            if (iter > 0) {
                result.convergence_rate = (prev_energy - energy) / prev_energy;
            }

            // Kung energy ay 0, satisfied ang lahat ng clauses
            if (energy < 1e-10) {
                result.satisfiable = true;
                break;
            }

            // Kung hindi na nagbabago, stuck tayo
            if (change < 1e-10 && iter > 10) {
                break;
            }

            prev_state = state;
            prev_energy = energy;
        }

        // Extract assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] > 0);
        }

        // Final check
        if (!result.satisfiable) {
            result.satisfiable = true;
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
                if (!sat) {
                    result.satisfiable = false;
                    break;
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenFixedPoint
