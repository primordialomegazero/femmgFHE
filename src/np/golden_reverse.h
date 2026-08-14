#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace GoldenReverse {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenReverseSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    // Reverse: imbis na hanapin kung ANO ang nagpapa-satisfy,
    // hanapin natin kung ANO ang nagpapa-UNSAT
    static std::vector<int> find_unsat_pattern(const std::vector<std::vector<int>>& clauses,
                                               int num_vars) {
        std::vector<int> var_occurs(num_vars, 0);
        std::vector<int> var_conflicts(num_vars, 0);

        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = abs(lit) - 1;
                var_occurs[var]++;
            }
        }

        // Ang "conflict" ay kapag ang isang variable ay lumalabas na positive at negative
        // sa magkaibang clauses na may overlapping variables
        for (int v = 0; v < num_vars; v++) {
            bool has_pos = false, has_neg = false;
            for (const auto& clause : clauses) {
                for (int lit : clause) {
                    if (abs(lit) - 1 == v) {
                        if (lit > 0) has_pos = true;
                        else has_neg = true;
                    }
                }
            }
            if (has_pos && has_neg) var_conflicts[v] = 1;
        }

        return var_conflicts;
    }

    // Reverse assignment: i-assign ang TRUE sa mga variables na may conflict
    static std::vector<int> reverse_assignment(const std::vector<std::vector<int>>& clauses,
                                               int num_vars) {
        std::vector<int> conflicts = find_unsat_pattern(clauses, num_vars);
        std::vector<int> state(num_vars, -1);  // Default FALSE

        // Ang mga variables na may conflict ay gawing TRUE
        for (int v = 0; v < num_vars; v++) {
            if (conflicts[v] == 1) {
                state[v] = 1;
            }
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

        // Reverse assignment
        std::vector<int> state = reverse_assignment(clauses, num_vars);

        std::vector<int> unsat_list;
        int unsat = count_unsat(clauses, state, &unsat_list);

        if (unsat == 0) {
            result.satisfiable = true;
        } else {
            // Reverse engineering: i-flip ang variables na WALA sa unsat clauses
            // kasi sila ang "pinaka-hindi-importante"
            for (int iteration = 0; iteration < num_vars && !result.satisfiable; iteration++) {
                result.steps = iteration + 2;

                // Hanapin ang variables na wala sa unsat clauses
                std::unordered_set<int> vars_in_unsat;
                for (int ci : unsat_list) {
                    for (int lit : clauses[ci]) {
                        vars_in_unsat.insert(abs(lit) - 1);
                    }
                }

                // I-flip ang variable na WALA sa unsat clauses
                int flip_var = -1;
                for (int v = 0; v < num_vars; v++) {
                    if (vars_in_unsat.find(v) == vars_in_unsat.end()) {
                        flip_var = v;
                        break;
                    }
                }

                if (flip_var == -1) {
                    // Kung lahat ng variables ay nasa unsat clauses,
                    // i-flip ang variable na may pinakamababang occurrence
                    std::vector<int> occurs(num_vars, 0);
                    for (const auto& clause : clauses) {
                        for (int lit : clause) {
                            occurs[abs(lit) - 1]++;
                        }
                    }
                    
                    int min_occurs = num_vars + 1;
                    for (int v = 0; v < num_vars; v++) {
                        if (occurs[v] < min_occurs) {
                            min_occurs = occurs[v];
                            flip_var = v;
                        }
                    }
                }

                if (flip_var == -1) break;

                state[flip_var] = 1 - state[flip_var];
                unsat = count_unsat(clauses, state, &unsat_list);

                if (unsat == 0) {
                    result.satisfiable = true;
                    break;
                }

                // Kung na-flip na natin ang maraming variables at wala pa ring solution,
                // subukan ang complement
                if (iteration > num_vars * 0.5) {
                    for (int v = 0; v < num_vars; v++) {
                        state[v] = 1 - state[v];  // I-complement lahat
                    }
                    unsat = count_unsat(clauses, state, &unsat_list);
                    
                    if (unsat == 0) {
                        result.satisfiable = true;
                        break;
                    }
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

} // namespace GoldenReverse
