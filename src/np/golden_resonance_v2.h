#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>

namespace GoldenResonanceV2 {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenResonanceV2Solver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double resonance_value;
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

    // Mas detalyadong resonance computation
    static double compute_resonance_v2(const std::vector<std::vector<int>>& clauses,
                                       int num_vars) {
        double resonance = 0.0;

        // 1. Mixed clause ratio (φ-1 = 0.618)
        int all_pos = 0, all_neg = 0, mixed = 0;
        int two_pos_one_neg = 0, one_pos_two_neg = 0;

        for (const auto& clause : clauses) {
            int pos_count = 0, neg_count = 0;
            for (int lit : clause) {
                if (lit > 0) pos_count++;
                else neg_count++;
            }

            if (pos_count == clause.size()) all_pos++;
            else if (neg_count == clause.size()) all_neg++;
            else {
                mixed++;
                if (pos_count == 2) two_pos_one_neg++;
                else one_pos_two_neg++;
            }
        }

        int total = clauses.size();
        double mixed_ratio = (double)mixed / total;
        double two_one_ratio = (double)two_pos_one_neg / (total + 1);
        double one_two_ratio = (double)one_pos_two_neg / (total + 1);

        // Kung mixed ratio ay malapit sa 0.618
        resonance += std::max(0.0, 1.0 - std::abs(mixed_ratio - 0.618) * 3.0);
        
        // Kung two_pos_one_neg at one_pos_two_neg ay balanse
        resonance += std::max(0.0, 1.0 - std::abs(two_one_ratio - one_two_ratio) * 5.0);

        // 2. Variable occurrence variance
        std::vector<int> var_occurs(num_vars, 0);
        for (const auto& clause : clauses) {
            for (int lit : clause) {
                var_occurs[abs(lit) - 1]++;
            }
        }

        double avg_occurs = 0;
        for (int v = 0; v < num_vars; v++) avg_occurs += var_occurs[v];
        avg_occurs /= num_vars;

        double variance = 0;
        for (int v = 0; v < num_vars; v++) {
            variance += (var_occurs[v] - avg_occurs) * (var_occurs[v] - avg_occurs);
        }
        variance /= num_vars;

        // Kung variance ay mababa (uniform distribution)
        resonance += std::max(0.0, 1.0 - (variance / (avg_occurs * avg_occurs)) * 2.0);

        // 3. Positive/negative balance
        int total_pos = 0, total_neg = 0;
        for (const auto& clause : clauses) {
            for (int lit : clause) {
                if (lit > 0) total_pos++;
                else total_neg++;
            }
        }

        double pos_ratio = (double)total_pos / (total_pos + total_neg);
        // Kung malapit sa 0.5 (balanced)
        resonance += std::max(0.0, 1.0 - std::abs(pos_ratio - 0.5) * 4.0);

        return resonance;
    }

    // Resonance-guided assignment
    static std::vector<int> resonance_assignment_v2(const std::vector<std::vector<int>>& clauses,
                                                    int num_vars) {
        std::vector<int> state(num_vars);
        
        // I-assign batay sa "golden frequency"
        std::vector<int> var_occurs(num_vars, 0);
        std::vector<int> var_pos(num_vars, 0);
        std::vector<int> var_neg(num_vars, 0);

        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = abs(lit) - 1;
                var_occurs[var]++;
                if (lit > 0) var_pos[var]++;
                else var_neg[var]++;
            }
        }

        for (int v = 0; v < num_vars; v++) {
            if (var_occurs[v] == 0) {
                state[v] = 1;
            } else {
                double pos_ratio = (double)var_pos[v] / var_occurs[v];
                // Kung balanced (near 0.5), gamitin ang golden ratio
                if (std::abs(pos_ratio - 0.5) < 0.15) {
                    state[v] = (v % 2 == 0) ? 1 : -1;
                } else {
                    state[v] = (pos_ratio > 0.5) ? 1 : -1;
                }
            }
        }

        return state;
    }

    // Refinement: golden local search
    static bool golden_refine(const std::vector<std::vector<int>>& clauses,
                              std::vector<int>& state,
                              int num_vars,
                              std::vector<int>& unsat_list,
                              int max_steps) {
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

            // Pumili ng variable mula sa unsat clause na may pinakamalaking impact
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

            if (best_var == -1) {
                // Random walk
                int ci = unsat_list[gen() % unsat_list.size()];
                int lit = clauses[ci][gen() % clauses[ci].size()];
                best_var = abs(lit) - 1;
                state[best_var] = 1 - state[best_var];
                current_unsat = count_unsat(clauses, state, &unsat_list);
            } else {
                state[best_var] = 1 - state[best_var];
                current_unsat = best_unsat;
                count_unsat(clauses, state, &unsat_list);
            }

            if (current_unsat == 0) return true;

            // Early termination kung stuck
            if (step > 50 && current_unsat > clauses.size() * 0.05) {
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
        result.resonance_value = 0.0;

        auto start = std::chrono::high_resolution_clock::now();

        // Compute resonance
        result.resonance_value = compute_resonance_v2(clauses, num_vars);

        // Kung may any resonance (value > 1.0), subukan ang assignment
        if (result.resonance_value > 1.0) {
            result.steps = 1;
            std::vector<int> state = resonance_assignment_v2(clauses, num_vars);
            std::vector<int> unsat_list;
            int unsat = count_unsat(clauses, state, &unsat_list);

            if (unsat == 0) {
                result.satisfiable = true;
            } else {
                // Refinement
                bool found = golden_refine(clauses, state, num_vars, unsat_list, 500);
                result.steps += 1;
                
                if (found) {
                    result.satisfiable = true;
                }
            }

            result.assignment.resize(num_vars);
            for (int v = 0; v < num_vars; v++) {
                result.assignment[v] = (state[v] == 1);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenResonanceV2
