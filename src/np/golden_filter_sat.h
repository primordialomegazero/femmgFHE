#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>

namespace GoldenFilterSAT {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenFilterSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    // Golden filter: hanapin ang "core" ng formula na nangangailangan ng atensyon
    static std::vector<int> golden_filter(const std::vector<std::vector<int>>& clauses,
                                          int num_vars) {
        std::vector<int> var_weight(num_vars, 0);
        std::vector<int> var_polarity(num_vars, 0);  // +1 positive, -1 negative

        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = abs(lit) - 1;
                var_weight[var]++;
                var_polarity[var] += (lit > 0) ? 1 : -1;
            }
        }

        // Golden assignment: kung ang polarity ay positive, TRUE; negative, FALSE
        std::vector<int> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            state[v] = (var_polarity[v] >= 0) ? 1 : -1;
        }

        return state;
    }

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

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 1;

        auto start = std::chrono::high_resolution_clock::now();

        // Golden filter assignment
        std::vector<int> state = golden_filter(clauses, num_vars);

        std::vector<int> unsat_list;
        int unsat = count_unsat(clauses, state, &unsat_list);

        if (unsat == 0) {
            result.satisfiable = true;
        } else {
            // Sa unsat clauses, i-flip ang variable na may pinakamalaking polarity
            for (int attempt = 0; attempt < 100 && !result.satisfiable; attempt++) {
                result.steps = attempt + 2;

                // Hanapin ang variable na may pinakamalaking impact sa unsat clauses
                std::unordered_map<int, int> impact;
                for (int ci : unsat_list) {
                    for (int lit : clauses[ci]) {
                        int var = abs(lit) - 1;
                        impact[var]++;
                    }
                }

                if (impact.empty()) break;

                // I-flip ang variable na may pinakamalaking impact
                int best_var = -1;
                int best_impact = 0;
                for (const auto& [var, count] : impact) {
                    if (count > best_impact) {
                        best_impact = count;
                        best_var = var;
                    }
                }

                if (best_var == -1) break;

                state[best_var] = 1 - state[best_var];
                unsat = count_unsat(clauses, state, &unsat_list);

                if (unsat == 0) {
                    result.satisfiable = true;
                    break;
                }

                // Kung hindi na bumababa, break
                if (attempt > 10 && unsat > clauses.size() * 0.01) {
                    break;
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

} // namespace GoldenFilterSAT
