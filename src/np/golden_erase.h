#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace GoldenErase {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenEraseSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long erased_clauses;
        double time_ms;
    };

private:
    // Erase approach: burahin ang satisfied clauses, i-analyze ang matitira
    static Result erase_and_emerge(const std::vector<std::vector<int>>& clauses,
                                   int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.erased_clauses = 0;

        // Gumawa ng working copy ng clauses
        std::vector<std::vector<int>> remaining = clauses;
        std::vector<int> state(num_vars, 0);  // 0 = unassigned

        // Statistics para sa initial assignment
        std::vector<int> pos_count(num_vars, 0);
        std::vector<int> neg_count(num_vars, 0);
        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (lit > 0) pos_count[var]++;
                else neg_count[var]++;
            }
        }

        // Initial assignment
        for (int v = 0; v < num_vars; v++) {
            state[v] = (pos_count[v] >= neg_count[v]) ? 1 : -1;
        }

        // Erase loop: burahin ang satisfied clauses hanggang may matira
        bool changed = true;
        while (changed && !remaining.empty()) {
            changed = false;
            result.steps++;

            // Hanapin ang satisfied clauses at burahin
            std::vector<std::vector<int>> new_remaining;
            for (const auto& clause : remaining) {
                bool sat = false;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (state[var] == 0) continue;
                    bool val = state[var] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        sat = true;
                        break;
                    }
                }
                if (!sat) {
                    new_remaining.push_back(clause);
                } else {
                    result.erased_clauses++;
                    changed = true;
                }
            }

            remaining = new_remaining;

            // Kung may natira pang clauses, i-update ang assignment
            if (!remaining.empty()) {
                // Hanapin ang variables na nasa remaining clauses
                std::unordered_set<int> vars_in_remaining;
                for (const auto& clause : remaining) {
                    for (int lit : clause) {
                        vars_in_remaining.insert(abs(lit) - 1);
                    }
                }

                // Para sa mga variables na wala na sa remaining, wala nang constraints
                // Para sa variables na nasa remaining, i-adjust ang assignment
                for (int v = 0; v < num_vars; v++) {
                    if (state[v] == 0 && vars_in_remaining.find(v) == vars_in_remaining.end()) {
                        // Wala nang constraints, i-assign freely
                        state[v] = 1;
                    }
                }

                // Kung may unit clause sa remaining, i-assign agad
                for (const auto& clause : remaining) {
                    int unassigned_count = 0;
                    int unassigned_var = -1;
                    int unassigned_sign = 0;
                    bool satisfied = false;

                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        if (state[var] == 0) {
                            unassigned_count++;
                            unassigned_var = var;
                            unassigned_sign = (lit > 0) ? 1 : -1;
                        } else {
                            bool val = state[var] == 1;
                            if ((lit > 0 && val) || (lit < 0 && !val)) {
                                satisfied = true;
                                break;
                            }
                        }
                    }

                    if (!satisfied && unassigned_count == 1) {
                        state[unassigned_var] = (unassigned_sign > 0) ? 1 : -1;
                        changed = true;
                    }
                }
            }
        }

        // Check kung may natira pang clauses
        if (remaining.empty()) {
            result.satisfiable = true;
        } else {
            // Subukan i-satisfy ang remaining clauses
            for (const auto& clause : remaining) {
                bool sat = false;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (state[var] == 0) {
                        state[var] = (lit > 0) ? 1 : -1;
                        sat = true;
                        break;
                    }
                }
            }

            // Final check
            bool all_sat = true;
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
                if (!sat) {
                    all_sat = false;
                    break;
                }
            }

            result.satisfiable = all_sat;
        }

        // I-extract ang assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            if (state[v] == 0) {
                result.assignment[v] = (pos_count[v] >= neg_count[v]);
            } else {
                result.assignment[v] = (state[v] == 1);
            }
        }

        return result;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars) {
        auto start = std::chrono::high_resolution_clock::now();
        Result result = erase_and_emerge(clauses, num_vars);
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return result;
    }
};

} // namespace GoldenErase
