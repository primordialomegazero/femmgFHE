#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <random>
#include <algorithm>
#include <unordered_map>

namespace GoldenEscape {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenEscapeSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long escapes;
        long long flips;
        double time_ms;
    };

private:
    // Compute unsatisfied clauses
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

    // Get unsatisfied clauses
    static std::vector<int> get_unsat_clauses(const std::vector<std::vector<int>>& clauses,
                                              const std::vector<int>& state) {
        std::vector<int> unsat_clauses;
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
            if (!sat) unsat_clauses.push_back(ci);
        }
        return unsat_clauses;
    }

    // Golden ratio escape: tumalon gamit ang φ pattern
    static std::vector<int> golden_escape(const std::vector<std::vector<int>>& clauses,
                                          const std::vector<int>& state,
                                          int num_vars,
                                          int escape_count) {
        std::vector<int> new_state = state;

        // Fibonacci sequence para sa escape pattern
        std::vector<int> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89};

        // Piliin ang k gaano karaming variables ang i-flip
        int k = std::min(escape_count + 1, (int)std::sqrt(num_vars) + 1);

        // Gamitin ang golden ratio para pumili ng variables
        std::vector<int> unsat_clauses = get_unsat_clauses(clauses, state);

        // Bilangin ang variable occurrences sa unsatisfied clauses
        std::unordered_map<int, int> var_count;
        for (int ci : unsat_clauses) {
            for (int lit : clauses[ci]) {
                int var = abs(lit) - 1;
                var_count[var]++;
            }
        }

        // I-sort ang variables by count
        std::vector<std::pair<int, int>> sorted_vars(var_count.begin(), var_count.end());
        std::sort(sorted_vars.begin(), sorted_vars.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });

        // Flip ang top k variables
        for (int i = 0; i < std::min(k, (int)sorted_vars.size()); i++) {
            int var = sorted_vars[i].first;
            new_state[var] = 1 - new_state[var];
        }

        // Kung kulang pa, random flip na may golden ratio spacing
        if (sorted_vars.size() < k) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, num_vars - 1);

            for (int i = sorted_vars.size(); i < k; i++) {
                int var = dis(gen);
                new_state[var] = 1 - new_state[var];
            }
        }

        return new_state;
    }

    // Local search na may tabu list
    static bool local_search(const std::vector<std::vector<int>>& clauses,
                             std::vector<int>& state,
                             int num_vars,
                             int max_flips,
                             long long& flips) {
        int current_unsat = count_unsat(clauses, state);

        std::vector<int> tabu(num_vars, 0);
        int tabu_tenure = (int)std::sqrt(num_vars) + 1;

        for (int flip = 0; flip < max_flips; flip++) {
            if (current_unsat == 0) return true;

            // Hanapin ang best non-tabu variable to flip
            int best_var = -1;
            int best_unsat = current_unsat + 1;  // Start with worse

            for (int v = 0; v < num_vars; v++) {
                if (tabu[v] > 0) continue;

                state[v] = 1 - state[v];  // Trial flip
                int new_unsat = count_unsat(clauses, state);
                state[v] = 1 - state[v];  // Revert

                if (new_unsat < best_unsat) {
                    best_unsat = new_unsat;
                    best_var = v;
                }
            }

            // Kung walang improving move, pumili ng random non-tabu
            if (best_var == -1 || best_unsat >= current_unsat) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, num_vars - 1);

                for (int attempts = 0; attempts < 100; attempts++) {
                    int v = dis(gen);
                    if (tabu[v] == 0) {
                        best_var = v;
                        break;
                    }
                }

                if (best_var == -1) {
                    // I-clear ang tabu list
                    std::fill(tabu.begin(), tabu.end(), 0);
                    continue;
                }
            }

            // Apply ang best flip
            state[best_var] = 1 - state[best_var];
            current_unsat = best_unsat;
            flips++;

            // Update tabu
            for (int v = 0; v < num_vars; v++) {
                if (tabu[v] > 0) tabu[v]--;
            }
            tabu[best_var] = tabu_tenure;

            if (current_unsat == 0) return true;
        }

        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars,
                        int max_steps = 10000) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.escapes = 0;
        result.flips = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Initialize with golden ratio based on variable counts
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
            state[v] = (neg_count[v] > pos_count[v]) ? 0 : 1;
        }

        int current_unsat = count_unsat(clauses, state);

        // Kung malas, subukan agad ang local search
        if (current_unsat > 0) {
            bool found = local_search(clauses, state, num_vars, 1000, result.flips);
            if (found) {
                result.satisfiable = true;
                result.steps = 1;
            } else {
                current_unsat = count_unsat(clauses, state);
            }
        } else {
            result.satisfiable = true;
            result.steps = 1;
        }

        // Kung hindi pa rin solved, mag-escape
        int escape_count = 0;
        while (!result.satisfiable && result.steps < max_steps) {
            result.steps++;

            // Subukan ang local search
            bool found = local_search(clauses, state, num_vars, 500, result.flips);
            if (found) {
                result.satisfiable = true;
                break;
            }

            // Golden escape
            state = golden_escape(clauses, state, num_vars, escape_count);
            result.escapes++;
            escape_count++;

            // Check kung solved na
            if (count_unsat(clauses, state) == 0) {
                result.satisfiable = true;
                break;
            }

            // Subukan muli ang local search pagkatapos ng escape
            found = local_search(clauses, state, num_vars, 500, result.flips);
            if (found) {
                result.satisfiable = true;
                break;
            }

            // I-reset kung sobrang tagal na
            if (escape_count > 20) {
                for (int v = 0; v < num_vars; v++) {
                    state[v] = (neg_count[v] > pos_count[v]) ? 0 : 1;
                }
                escape_count = 0;
                result.escapes++;
            }
        }

        // Extract assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] == 1);
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenEscape
