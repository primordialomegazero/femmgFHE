#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace GoldenInfinite {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenInfiniteSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long infinite_steps;
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

    // Infinite recursion: i-solve ang clause, tapos i-propagate ang consequences
    // φ = 1 + 1/(1 + 1/(1 + ...)) — ang bawat level ay nagre-refine
    static bool infinite_propagate(const std::vector<std::vector<int>>& clauses,
                                   std::vector<int>& state,
                                   int num_vars,
                                   int depth,
                                   int max_depth,
                                   long long& infinite_steps) {
        if (depth >= max_depth) return false;

        std::vector<int> unsat_list;
        int current_unsat = count_unsat(clauses, state, &unsat_list);

        if (current_unsat == 0) return true;

        // Sa bawat depth, i-solve ang isang unsat clause
        if (!unsat_list.empty()) {
            int ci = unsat_list[0];  // Kunin ang unang unsat clause
            
            // Subukan i-satisfy ang clause na ito
            for (int lit : clauses[ci]) {
                int var = abs(lit) - 1;
                int original = state[var];
                
                // I-assign para ma-satisfy ang clause
                state[var] = (lit > 0) ? 1 : -1;
                infinite_steps++;
                
                // Recursive: i-propagate ang consequences
                if (infinite_propagate(clauses, state, num_vars, depth + 1, max_depth, infinite_steps)) {
                    return true;
                }
                
                // Backtrack kung hindi gumana
                state[var] = original;
            }
        }

        return false;
    }

    // Golden ratio unfolding: 1, 1+1/1, 1+1/(1+1/1), ...
    static std::vector<double> golden_unfolding(int levels) {
        std::vector<double> values;
        double current = 1.0;
        
        for (int i = 0; i < levels; i++) {
            values.push_back(current);
            current = 1.0 + 1.0 / current;
        }
        
        return values;
    }

    // Infinite cascade: gamitin ang golden unfolding para sa variable selection
    static bool infinite_cascade(const std::vector<std::vector<int>>& clauses,
                                 std::vector<int>& state,
                                 int num_vars,
                                 int max_steps,
                                 long long& infinite_steps) {
        auto golden_values = golden_unfolding(20);
        
        std::vector<int> unsat_list;
        int current_unsat = count_unsat(clauses, state, &unsat_list);
        if (current_unsat == 0) return true;

        for (int step = 0; step < max_steps; step++) {
            if (unsat_list.empty()) {
                current_unsat = count_unsat(clauses, state, &unsat_list);
                if (current_unsat == 0) return true;
            }

            // Pumili ng variable gamit ang golden ratio
            double golden = golden_values[step % golden_values.size()];
            int var_index = (int)(golden * num_vars) % num_vars;
            
            // Hanapin ang variable na may pinakamalaking impact sa unsat clauses
            std::unordered_map<int, int> impact;
            for (int ci : unsat_list) {
                for (int lit : clauses[ci]) {
                    impact[abs(lit) - 1]++;
                }
            }

            int best_var = var_index;
            int best_impact = 0;
            for (const auto& [var, count] : impact) {
                if (count > best_impact) {
                    best_impact = count;
                    best_var = var;
                }
            }

            // I-flip ang variable
            state[best_var] = 1 - state[best_var];
            infinite_steps++;
            
            current_unsat = count_unsat(clauses, state, &unsat_list);
            
            if (current_unsat == 0) return true;
        }

        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.infinite_steps = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Initial assignment
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
            state[v] = (pos_count[v] >= neg_count[v]) ? 1 : -1;
        }

        std::vector<int> unsat_list;
        int current_unsat = count_unsat(clauses, state, &unsat_list);

        if (current_unsat == 0) {
            result.satisfiable = true;
            result.steps = 1;
        } else {
            // Subukan ang infinite cascade
            bool found = infinite_cascade(clauses, state, num_vars, 500, result.infinite_steps);
            result.steps = 2;

            if (found) {
                result.satisfiable = true;
            } else {
                // Infinite propagation na may backtracking
                found = infinite_propagate(clauses, state, num_vars, 0, 10, result.infinite_steps);
                result.steps = 3;

                if (found) {
                    result.satisfiable = true;
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

} // namespace GoldenInfinite
