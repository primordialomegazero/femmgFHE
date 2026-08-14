#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <random>

namespace GoldenClauseSignature {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenClauseSignatureSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double signature_value;
        double time_ms;
    };

private:
    // Compute ang clause-based signature
    static double compute_clause_signature(const std::vector<std::vector<int>>& clauses,
                                           int num_vars) {
        // 1. Clause polarity distribution
        int all_pos = 0, all_neg = 0, mixed = 0;
        int two_pos_one_neg = 0, one_pos_two_neg = 0;

        for (const auto& clause : clauses) {
            int pos_count = 0, neg_count = 0;
            for (int lit : clause) {
                if (lit > 0) pos_count++;
                else neg_count++;
            }

            if (pos_count == 3) all_pos++;
            else if (neg_count == 3) all_neg++;
            else {
                mixed++;
                if (pos_count == 2) two_pos_one_neg++;
                else one_pos_two_neg++;
            }
        }

        int total = clauses.size();
        double all_pos_ratio = (double)all_pos / total;
        double all_neg_ratio = (double)all_neg / total;
        double mixed_ratio = (double)mixed / total;

        // 2. Variable occurrence pattern
        std::vector<int> var_count(num_vars, 0);
        for (const auto& clause : clauses) {
            for (int lit : clause) {
                var_count[abs(lit) - 1]++;
            }
        }

        double avg_occurrence = 0;
        double var_std = 0;
        for (int v = 0; v < num_vars; v++) {
            avg_occurrence += var_count[v];
        }
        avg_occurrence /= num_vars;

        for (int v = 0; v < num_vars; v++) {
            var_std += (var_count[v] - avg_occurrence) * (var_count[v] - avg_occurrence);
        }
        var_std = std::sqrt(var_std / num_vars);

        // 3. Golden signature: kombinasyon ng clause polarity at variance
        // Sa satisfiable formulas, may balance sa polarity
        double polarity_balance = std::abs(all_pos_ratio - all_neg_ratio);
        double golden_term = std::abs(mixed_ratio - 0.618);  // φ - 1

        // Signature: mas mababa = mas satisfiable
        double signature = polarity_balance + golden_term + (var_std / avg_occurrence);

        return signature;
    }

    // Direct assignment mula sa clause structure
    static std::vector<int> clause_based_assignment(const std::vector<std::vector<int>>& clauses,
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

        // Kung mas maraming positive, TRUE. Kung mas maraming negative, FALSE.
        std::vector<int> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            if (pos_count[v] >= neg_count[v]) state[v] = 1;
            else state[v] = -1;
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

        auto start = std::chrono::high_resolution_clock::now();

        // Compute clause signature
        result.signature_value = compute_clause_signature(clauses, num_vars);

        // Direct assignment
        std::vector<int> state = clause_based_assignment(clauses, num_vars);
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

            for (int flip = 0; flip < 500; flip++) {
                int best_var = -1;
                int best_unsat = unsat + 1;

                for (int s = 0; s < std::min(num_vars, 25); s++) {
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

                // Early termination kung walang improvement sa 50 flips
                if (flip > 50 && best_unsat >= unsat) break;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenClauseSignature
