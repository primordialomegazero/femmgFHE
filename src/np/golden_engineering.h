#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>

namespace GoldenEngineering {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenEngineeringSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
        std::string strategy_used;
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

    // COMPONENT 1: Entanglement-based initialization
    static std::vector<int> entanglement_init(const std::vector<std::vector<int>>& clauses,
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
            if (pos_count[v] + neg_count[v] == 0) {
                state[v] = 1;
            } else {
                double ratio = (double)neg_count[v] / (pos_count[v] + neg_count[v]);
                if (std::abs(ratio - 0.618) < 0.1) {
                    state[v] = (v % 2 == 0) ? 1 : -1;
                } else {
                    state[v] = (ratio > 0.618) ? -1 : 1;
                }
            }
        }
        return state;
    }

    // COMPONENT 2: Propagation na may unit clause detection
    static bool propagate_unit_clauses(const std::vector<std::vector<int>>& clauses,
                                       std::vector<int>& state,
                                       std::vector<int>& unsat_list) {
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (const auto& clause : clauses) {
                int unassigned_count = 0;
                int unassigned_var = -1;
                int unassigned_sign = 0;
                bool satisfied = false;

                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    bool val = state[var] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        satisfied = true;
                        break;
                    }
                    if (state[var] == 0) {
                        unassigned_count++;
                        unassigned_var = var;
                        unassigned_sign = (lit > 0) ? 1 : -1;
                    }
                }

                if (!satisfied && unassigned_count == 1) {
                    state[unassigned_var] = (unassigned_sign > 0) ? 1 : -1;
                    changed = true;
                }
            }
        }

        return count_unsat(clauses, state, &unsat_list) == 0;
    }

    // COMPONENT 3: Golden local search
    static bool golden_local_search(const std::vector<std::vector<int>>& clauses,
                                    std::vector<int>& state,
                                    int num_vars,
                                    int max_steps) {
        std::vector<int> unsat_list;
        int current_unsat = count_unsat(clauses, state, &unsat_list);
        if (current_unsat == 0) return true;

        std::random_device rd;
        std::mt19937 gen(42);
        std::uniform_int_distribution<> dis(0, num_vars - 1);

        for (int step = 0; step < max_steps; step++) {
            if (unsat_list.empty()) {
                current_unsat = count_unsat(clauses, state, &unsat_list);
                if (current_unsat == 0) return true;
            }

            // I-flip ang variable na may pinakamalaking impact
            std::unordered_map<int, int> impact;
            for (int ci : unsat_list) {
                for (int lit : clauses[ci]) {
                    impact[abs(lit) - 1]++;
                }
            }

            if (impact.empty()) break;

            int best_var = -1;
            int best_unsat = current_unsat + 1;

            for (const auto& [var, _] : impact) {
                state[var] = 1 - state[var];
                int new_unsat = count_unsat(clauses, state);
                state[var] = 1 - state[var];

                if (new_unsat < best_unsat) {
                    best_unsat = new_unsat;
                    best_var = var;
                }
            }

            if (best_var == -1) break;

            state[best_var] = 1 - state[best_var];
            current_unsat = best_unsat;
            count_unsat(clauses, state, &unsat_list);

            if (current_unsat == 0) return true;

            // Early termination
            if (step > 100 && current_unsat > clauses.size() * 0.02) {
                return false;
            }
        }

        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.strategy_used = "none";

        auto start = std::chrono::high_resolution_clock::now();

        // Stage 1: Entanglement initialization
        std::vector<int> state = entanglement_init(clauses, num_vars);
        result.strategy_used = "entanglement";

        std::vector<int> unsat_list;
        int unsat = count_unsat(clauses, state, &unsat_list);

        if (unsat == 0) {
            result.satisfiable = true;
            result.steps = 1;
        } else {
            // Stage 2: Unit clause propagation
            bool found = propagate_unit_clauses(clauses, state, unsat_list);
            result.strategy_used = "propagation";
            result.steps = 2;

            if (found) {
                result.satisfiable = true;
            } else {
                // Stage 3: Golden local search
                found = golden_local_search(clauses, state, num_vars, 300);
                result.strategy_used = "local_search";
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

} // namespace GoldenEngineering
