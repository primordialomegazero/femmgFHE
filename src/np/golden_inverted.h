#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace GoldenInverted {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenInvertedSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    static int count_unsat(const std::vector<std::vector<int>>& clauses,
                           const std::vector<int>& state,
                           std::vector<int>* unsat_list = nullptr) {
        int unsat = 0;
        if (unsat_list) unsat_list->clear();

        for (int ci = 0; ci < clauses.size(); ci++) {
            bool sat = false;
            for (int lit : clauses[ci]) {
                int var = abs(lit) - 1;
                bool val = state[var] == 1;
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    sat = true;
                    break;
                }
            }
            if (!sat) {
                unsat++;
                if (unsat_list) unsat_list->push_back(ci);
            }
        }
        return unsat;
    }

    // Inverted approach: hanapin ang variables na PINAKA-PROBLEMATIC
    // at i-flip sila sa kabaligtaran ng polarity
    static std::vector<int> inverted_assignment(const std::vector<std::vector<int>>& clauses,
                                                int num_vars) {
        std::vector<int> var_polarity(num_vars, 0);
        std::vector<int> var_unsat_count(num_vars, 0);

        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = abs(lit) - 1;
                var_polarity[var] += (lit > 0) ? 1 : -1;
            }
        }

        // Inverted: kung positive polarity, gawing FALSE (baligtad)
        // kasi baka ito ang nagdudulot ng conflict
        std::vector<int> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            state[v] = (var_polarity[v] >= 0) ? -1 : 1;  // INVERTED!
        }

        return state;
    }

    // Compute ang "conflict score" ng bawat variable
    static std::vector<int> conflict_scores(const std::vector<std::vector<int>>& clauses,
                                            const std::vector<int>& state,
                                            int num_vars) {
        std::vector<int> scores(num_vars, 0);

        std::vector<int> unsat_list;
        count_unsat(clauses, state, &unsat_list);

        for (int ci : unsat_list) {
            for (int lit : clauses[ci]) {
                int var = abs(lit) - 1;
                scores[var]++;
            }
        }

        return scores;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 1;

        auto start = std::chrono::high_resolution_clock::now();

        // Inverted initial assignment
        std::vector<int> state = inverted_assignment(clauses, num_vars);

        std::vector<int> unsat_list;
        int unsat = count_unsat(clauses, state, &unsat_list);

        if (unsat == 0) {
            result.satisfiable = true;
        } else {
            // Iteratively i-flip ang variable na may pinakamataas na conflict score
            std::unordered_set<int> flipped_vars;

            for (int iteration = 0; iteration < num_vars && !result.satisfiable; iteration++) {
                result.steps = iteration + 2;

                // Compute conflict scores
                std::vector<int> scores = conflict_scores(clauses, state, num_vars);

                // Hanapin ang variable na may pinakamataas na score at hindi pa na-flip
                int best_var = -1;
                int best_score = 0;
                for (int v = 0; v < num_vars; v++) {
                    if (flipped_vars.find(v) == flipped_vars.end() && scores[v] > best_score) {
                        best_score = scores[v];
                        best_var = v;
                    }
                }

                if (best_var == -1) break;

                // I-flip ang variable
                state[best_var] = 1 - state[best_var];
                flipped_vars.insert(best_var);

                // Check ulit
                unsat = count_unsat(clauses, state, &unsat_list);

                if (unsat == 0) {
                    result.satisfiable = true;
                    break;
                }

                // Kung na-flip na natin ang 10% ng variables at wala pa ring improvement,
                // mag-switch sa inverse ng initial assignment
                if (flipped_vars.size() > num_vars * 0.1) {
                    state = inverted_assignment(clauses, num_vars);
                    for (int v : flipped_vars) {
                        state[v] = 1 - state[v];
                    }
                    unsat = count_unsat(clauses, state, &unsat_list);
                    
                    if (unsat == 0) {
                        result.satisfiable = true;
                    }
                    flipped_vars.clear();
                }
            }
        }

        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] == 1);
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenInverted
