#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <random>
#include <algorithm>

namespace GoldenDiscreteCollapse {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenDiscreteSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long flips;
        double time_ms;
    };

private:
    // Clause energy na discrete (hindi continuous)
    static int clause_energy_discrete(const std::vector<int>& clause,
                                      const std::vector<int>& state_binary) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            bool val = state_binary[var] == 1;
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                return 0;  // Satisfied
            }
        }
        return 1;  // Unsatisfied
    }

    // Total unsatisfied clauses
    static int total_unsatisfied(const std::vector<std::vector<int>>& clauses,
                                 const std::vector<int>& state_binary) {
        int count = 0;
        for (const auto& clause : clauses) {
            count += clause_energy_discrete(clause, state_binary);
        }
        return count;
    }

    // Aling variable ang pinaka-makakatulong kung i-flip?
    static int best_flip_variable(const std::vector<std::vector<int>>& clauses,
                                  const std::vector<int>& state_binary,
                                  int num_vars) {
        int best_var = -1;
        int best_improvement = 0;
        int current_unsat = total_unsatisfied(clauses, state_binary);

        // Check bawat variable flip
        for (int v = 0; v < num_vars; v++) {
            std::vector<int> flipped = state_binary;
            flipped[v] = 1 - flipped[v];  // Flip

            int new_unsat = total_unsatisfied(clauses, flipped);
            int improvement = current_unsat - new_unsat;

            if (improvement > best_improvement) {
                best_improvement = improvement;
                best_var = v;
            }
        }

        return best_var;
    }

    // Golden ratio scoring para sa variable
    static double golden_score(const std::vector<std::vector<int>>& clauses,
                               const std::vector<int>& state_binary,
                               int var) {
        // Bilangin kung gaano karaming unsatisfied clauses ang may literal sa var na ito
        int unsat_with_var = 0;
        int sat_with_var = 0;

        for (const auto& clause : clauses) {
            bool unsat = true;
            bool has_var = false;

            for (int lit : clause) {
                int v = abs(lit) - 1;
                if (v == var) has_var = true;

                bool val = state_binary[v] == 1;
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    unsat = false;
                    break;
                }
            }

            if (has_var) {
                if (unsat) unsat_with_var++;
                else sat_with_var++;
            }
        }

        // Golden ratio balance: dapat balanse ang positive at negative impact
        if (unsat_with_var + sat_with_var == 0) return 0.0;
        return (double)(unsat_with_var - sat_with_var) / (unsat_with_var + sat_with_var);
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars,
                        int max_steps = 10000) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.flips = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Initialize with golden ratio pattern
        std::vector<int> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            state[v] = (v % 2 == 0) ? 1 : 0;  // PHI : PSI
        }

        int current_unsat = total_unsatisfied(clauses, state);
        int best_unsat = current_unsat;
        std::vector<int> best_state = state;

        // Discrete hill climbing with golden ratio guidance
        for (int step = 0; step < max_steps; step++) {
            result.steps = step + 1;

            if (current_unsat == 0) {
                result.satisfiable = true;
                break;
            }

            // Hanapin ang best variable na i-flip
            int flip_var = best_flip_variable(clauses, state, num_vars);

            if (flip_var == -1) {
                // Stuck sa local minima, subukan ang golden ratio scoring
                std::vector<double> scores(num_vars);
                for (int v = 0; v < num_vars; v++) {
                    scores[v] = golden_score(clauses, state, v);
                }

                // Piliin ang variable na may pinaka-mataas na score
                auto max_it = std::max_element(scores.begin(), scores.end());
                flip_var = std::distance(scores.begin(), max_it);

                if (*max_it == 0.0) {
                    // Random restart na may golden influence
                    for (int v = 0; v < num_vars; v++) {
                        state[v] = (v % 2 == 0) ? 1 : 0;
                    }
                    current_unsat = total_unsatisfied(clauses, state);
                    continue;
                }
            }

            // Flip ang variable
            state[flip_var] = 1 - state[flip_var];
            result.flips++;

            int new_unsat = total_unsatisfied(clauses, state);

            // Kung mas maganda, i-save
            if (new_unsat < best_unsat) {
                best_unsat = new_unsat;
                best_state = state;
            }

            current_unsat = new_unsat;

            // Kung stuck na sa local minima, subukan ang tabu-like escape
            if (new_unsat >= current_unsat && step > 100) {
                // Subukan ang random flip ng 2 variables
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, num_vars - 1);

                int var1 = dis(gen);
                int var2 = dis(gen);
                while (var2 == var1) var2 = dis(gen);

                state[var1] = 1 - state[var1];
                state[var2] = 1 - state[var2];
                result.flips += 2;
                current_unsat = total_unsatisfied(clauses, state);
            }
        }

        // Extract best assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (best_state[v] == 1);
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenDiscreteCollapse
