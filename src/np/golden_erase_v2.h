#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace GoldenEraseV2 {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenEraseSolverV2 {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long erased_clauses;
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
                if (state[var] == 0) continue;
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

    // Erase na may look-ahead: bago burahin, tignan ang epekto
    static Result erase_with_lookahead(const std::vector<std::vector<int>>& clauses,
                                       int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.erased_clauses = 0;

        std::vector<int> state(num_vars, 0);

        // Initial assignment na mas matalino
        std::vector<int> pos_count(num_vars, 0);
        std::vector<int> neg_count(num_vars, 0);
        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (lit > 0) pos_count[var]++;
                else neg_count[var]++;
            }
        }

        for (int v = 0; v < num_vars; v++) {
            state[v] = (pos_count[v] >= neg_count[v]) ? 1 : -1;
        }

        std::vector<int> unsat_list;
        int current_unsat = count_unsat(clauses, state, &unsat_list);

        // Kung konti lang ang unsat, i-target agad
        if (current_unsat == 0) {
            result.satisfiable = true;
            result.assignment.resize(num_vars);
            for (int v = 0; v < num_vars; v++) {
                result.assignment[v] = (state[v] == 1);
            }
            return result;
        }

        // Iterative refinement: i-flip lang ang variables na nasa unsat clauses
        std::unordered_set<int> flipped;
        int max_iterations = std::min(num_vars * 2, 200);

        for (int iter = 0; iter < max_iterations && !result.satisfiable; iter++) {
            result.steps = iter + 1;

            // Hanapin ang variable na may pinakamalaking impact sa unsat clauses
            std::unordered_map<int, int> impact;
            for (int ci : unsat_list) {
                for (int lit : clauses[ci]) {
                    int var = abs(lit) - 1;
                    impact[var]++;
                }
            }

            if (impact.empty()) {
                current_unsat = count_unsat(clauses, state, &unsat_list);
                if (current_unsat == 0) {
                    result.satisfiable = true;
                    break;
                }
                if (unsat_list.empty()) {
                    result.satisfiable = true;
                    break;
                }
                continue;
            }

            // I-flip ang variable na may pinakamalaking impact at hindi pa na-flip
            int best_var = -1;
            int best_impact = 0;
            for (const auto& [var, count] : impact) {
                if (flipped.find(var) == flipped.end() && count > best_impact) {
                    best_impact = count;
                    best_var = var;
                }
            }

            if (best_var == -1) {
                // I-clear ang flipped set at subukan muli
                flipped.clear();
                continue;
            }

            // Trial flip
            state[best_var] = 1 - state[best_var];
            int new_unsat = count_unsat(clauses, state, &unsat_list);

            if (new_unsat < current_unsat) {
                // Magandang flip, i-keep
                current_unsat = new_unsat;
                flipped.insert(best_var);
                result.erased_clauses++;

                if (current_unsat == 0) {
                    result.satisfiable = true;
                    break;
                }
            } else {
                // I-revert kung hindi nakatulong
                state[best_var] = 1 - state[best_var];
                flipped.insert(best_var);  // Mark as tried
            }

            // Kung marami nang na-flip at wala pa ring improvement,
            // subukan ang complement
            if (flipped.size() > num_vars * 0.3) {
                for (int v = 0; v < num_vars; v++) {
                    state[v] = 1 - state[v];
                }
                current_unsat = count_unsat(clauses, state, &unsat_list);
                flipped.clear();

                if (current_unsat == 0) {
                    result.satisfiable = true;
                    break;
                }
            }
        }

        // Final check
        int final_unsat = count_unsat(clauses, state);
        result.satisfiable = (final_unsat == 0);

        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] == 1);
        }

        return result;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars) {
        auto start = std::chrono::high_resolution_clock::now();
        Result result = erase_with_lookahead(clauses, num_vars);
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return result;
    }
};

} // namespace GoldenEraseV2
