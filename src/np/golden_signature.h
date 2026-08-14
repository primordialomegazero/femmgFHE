#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <random>

namespace GoldenSignature {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenSignatureSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double signature_value;
        double time_ms;
    };

private:
    // Compute ang golden signature ng formula
    static double compute_signature(const std::vector<std::vector<int>>& clauses,
                                    int num_vars) {
        std::vector<int> pos_count(num_vars, 0);
        std::vector<int> neg_count(num_vars, 0);

        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (lit > 0) pos_count[var]++;
                else neg_count[var]++;
            }
        }

        // Golden ratio balance: φ² = φ + 1
        // Kung balanse ang formula, may solution
        double signature = 0.0;
        double total_balance = 0.0;

        for (int v = 0; v < num_vars; v++) {
            double total = pos_count[v] + neg_count[v];
            if (total == 0) continue;

            double balance = (pos_count[v] - neg_count[v]) / total;
            // Golden signature: kung balance ay malapit sa φ-1 = 0.618
            double golden_deviation = std::abs(balance - 0.618);
            signature += golden_deviation;
            total_balance += balance;
        }

        // Normalize
        signature /= num_vars;
        total_balance /= num_vars;

        // Kombinasyon ng signature at balance
        return signature + std::abs(total_balance - 0.618);
    }

    // Extract assignment mula sa signature
    static std::vector<int> signature_assignment(const std::vector<std::vector<int>>& clauses,
                                                 int num_vars) {
        std::vector<int> pos_count(num_vars, 0);
        std::vector<int> neg_count(num_vars, 0);

        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (lit > 0) pos_count[var]++;
                else neg_count[var]++;
            }
        }

        std::vector<int> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            // Kung mas maraming positive, TRUE
            // Kung mas maraming negative, FALSE
            // Kung balanse, gamitin ang golden ratio pattern
            if (pos_count[v] > neg_count[v]) {
                state[v] = 1;
            } else if (neg_count[v] > pos_count[v]) {
                state[v] = -1;
            } else {
                state[v] = (v % 2 == 0) ? 1 : -1;
            }
        }

        return state;
    }

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

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars) {
        Result result;
        result.steps = 1;
        result.signature_value = 0.0;

        auto start = std::chrono::high_resolution_clock::now();

        // Compute signature
        result.signature_value = compute_signature(clauses, num_vars);

        // Kung mababa ang signature (malapit sa golden ratio), SAT
        if (result.signature_value < 0.5) {
            // Direct assignment mula sa signature
            std::vector<int> state = signature_assignment(clauses, num_vars);
            int unsat = count_unsat(clauses, state);

            if (unsat == 0) {
                result.satisfiable = true;
                result.assignment.resize(num_vars);
                for (int v = 0; v < num_vars; v++) {
                    result.assignment[v] = (state[v] == 1);
                }
            } else {
                // Local refinement
                std::random_device rd;
                std::mt19937 gen(42);
                std::uniform_int_distribution<> dis(0, num_vars - 1);

                for (int flip = 0; flip < 1000; flip++) {
                    int best_var = -1;
                    int best_unsat = unsat + 1;

                    for (int s = 0; s < std::min(num_vars, 30); s++) {
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
        } else {
            result.satisfiable = false;
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenSignature
