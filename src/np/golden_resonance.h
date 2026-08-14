#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace GoldenResonance {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenResonanceSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double resonance_value;
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

    // Compute ang "golden resonance" ng formula
    static double compute_resonance(const std::vector<std::vector<int>>& clauses,
                                    int num_vars) {
        // 1. Clause length distribution
        std::unordered_map<int, int> length_count;
        for (const auto& clause : clauses) {
            length_count[clause.size()]++;
        }

        // 2. Variable occurrence pattern
        std::vector<int> var_occurs(num_vars, 0);
        for (const auto& clause : clauses) {
            for (int lit : clause) {
                var_occurs[abs(lit) - 1]++;
            }
        }

        // 3. Golden resonance: hanapin ang pattern na φ, φ², φ³...
        double resonance = 0.0;
        
        // Check kung ang clause lengths ay sumusunod sa Fibonacci
        for (const auto& [len, count] : length_count) {
            // Ang "ideal" distribution ay Fibonacci-like
            double golden_ratio = (double)count / clauses.size();
            // Kung malapit sa 1/φ = 0.618 o 1/φ² = 0.382
            if (std::abs(golden_ratio - 0.618) < 0.2 ||
                std::abs(golden_ratio - 0.382) < 0.2) {
                resonance += 1.0;
            }
        }

        // Check ang variable occurrence distribution
        double avg_occurs = 0;
        for (int v = 0; v < num_vars; v++) {
            avg_occurs += var_occurs[v];
        }
        avg_occurs /= num_vars;

        // Kung ang average occurrence ay malapit sa φ, may resonance
        double golden_occurs = 3.0 * PHI;  // 3 clauses * φ ≈ 4.854
        if (std::abs(avg_occurs - golden_occurs) < 1.0) {
            resonance += 1.0;
        }

        // 4. Golden ratio sa clause polarity
        int all_pos = 0, all_neg = 0, mixed = 0;
        for (const auto& clause : clauses) {
            int pos_count = 0, neg_count = 0;
            for (int lit : clause) {
                if (lit > 0) pos_count++;
                else neg_count++;
            }
            if (pos_count == clause.size()) all_pos++;
            else if (neg_count == clause.size()) all_neg++;
            else mixed++;
        }

        int total = clauses.size();
        double mixed_ratio = (double)mixed / total;
        
        // Kung ang mixed ratio ay malapit sa φ-1 = 0.618
        if (std::abs(mixed_ratio - 0.618) < 0.2) {
            resonance += 1.0;
        }

        return resonance;
    }

    // Golden resonance-based assignment
    static std::vector<int> resonance_assignment(const std::vector<std::vector<int>>& clauses,
                                                 int num_vars) {
        std::vector<int> state(num_vars);
        
        // I-assign batay sa "golden frequency"
        std::vector<int> var_occurs(num_vars, 0);
        for (const auto& clause : clauses) {
            for (int lit : clause) {
                var_occurs[abs(lit) - 1]++;
            }
        }

        // Ang variables na may occurrence na malapit sa φ ay TRUE
        for (int v = 0; v < num_vars; v++) {
            double normalized = (double)var_occurs[v] / 3.0;  // Divide by clause length
            // Kung malapit sa φ, TRUE
            if (std::abs(normalized - PHI) < 1.5) {
                state[v] = 1;
            } else {
                state[v] = -1;
            }
        }

        return state;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 1;
        result.resonance_value = 0.0;

        auto start = std::chrono::high_resolution_clock::now();

        // Compute resonance
        result.resonance_value = compute_resonance(clauses, num_vars);

        // Kung may resonance (value > 1.5), subukan ang resonance assignment
        if (result.resonance_value > 1.5) {
            std::vector<int> state = resonance_assignment(clauses, num_vars);
            int unsat = count_unsat(clauses, state);

            if (unsat == 0) {
                result.satisfiable = true;
                result.assignment.resize(num_vars);
                for (int v = 0; v < num_vars; v++) {
                    result.assignment[v] = (state[v] == 1);
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenResonance
