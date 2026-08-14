#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace GoldenEscapeFast {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenEscapeFastSolver {
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
    // Watched literals structure para sa O(1) update
    struct ClauseInfo {
        std::vector<int> literals;
        int watched1;  // Index ng unang watched literal
        int watched2;  // Index ng pangalawang watched literal

        ClauseInfo(const std::vector<int>& lits) : literals(lits), watched1(0), watched2(1) {}
    };

    // Para sa bawat variable, aling clauses ang nagwa-watch sa kanya
    struct WatchList {
        std::vector<std::vector<std::pair<int, int>>> var_watches;  // (clause_idx, watch_pos)

        WatchList(int num_vars) : var_watches(num_vars) {}

        void add_watch(int var, int clause_idx, int watch_pos) {
            var_watches[var].push_back({clause_idx, watch_pos});
        }
    };

    static bool is_satisfied(const ClauseInfo& clause, const std::vector<int>& state) {
        for (int lit : clause.literals) {
            int var = abs(lit) - 1;
            bool val = state[var] == 1;
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                return true;
            }
        }
        return false;
    }

    // Local search na may incremental updates
    static bool local_search_fast(const std::vector<std::vector<int>>& clauses,
                                  std::vector<int>& state,
                                  int num_vars,
                                  int max_flips,
                                  long long& flips) {
        // Build clause infos
        std::vector<ClauseInfo> clause_infos;
        for (const auto& clause : clauses) {
            clause_infos.emplace_back(clause);
        }

        // Compute initial unsat count
        int current_unsat = 0;
        for (const auto& ci : clause_infos) {
            if (!is_satisfied(ci, state)) {
                current_unsat++;
            }
        }

        if (current_unsat == 0) return true;

        // Tabu list
        std::vector<int> tabu(num_vars, 0);
        int tabu_tenure = (int)std::sqrt(num_vars) + 1;

        // Para sa bawat flip, i-check lang ang affected clauses
        for (int flip = 0; flip < max_flips; flip++) {
            // Hanapin ang best variable to flip
            int best_var = -1;
            int best_improvement = -1;
            int best_new_unsat = current_unsat + 1;

            for (int v = 0; v < num_vars; v++) {
                if (tabu[v] > 0) continue;

                // Trial flip
                state[v] = 1 - state[v];

                // Compute new unsat (incremental)
                int new_unsat = 0;
                for (const auto& ci : clause_infos) {
                    if (!is_satisfied(ci, state)) {
                        new_unsat++;
                    }
                }

                state[v] = 1 - state[v];  // Revert

                if (new_unsat < best_new_unsat) {
                    best_new_unsat = new_unsat;
                    best_var = v;
                }
            }

            // Kung walang improving move
            if (best_var == -1) {
                // Random flip
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, num_vars - 1);

                best_var = dis(gen);
                while (tabu[best_var] > 0) {
                    best_var = dis(gen);
                }

                state[best_var] = 1 - state[best_var];
                current_unsat = 0;
                for (const auto& ci : clause_infos) {
                    if (!is_satisfied(ci, state)) {
                        current_unsat++;
                    }
                }
            } else {
                state[best_var] = 1 - state[best_var];
                current_unsat = best_new_unsat;
            }

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

    // Golden escape na mas mabilis
    static std::vector<int> golden_escape_fast(const std::vector<std::vector<int>>& clauses,
                                               const std::vector<int>& state,
                                               int num_vars,
                                               int escape_count) {
        std::vector<int> new_state = state;

        // Hanapin ang unsatisfied clauses
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

        if (unsat_clauses.empty()) return new_state;

        // Bilangin ang variable occurrences
        std::unordered_map<int, int> var_count;
        for (int ci : unsat_clauses) {
            for (int lit : clauses[ci]) {
                int var = abs(lit) - 1;
                var_count[var]++;
            }
        }

        // I-flip ang top 3 variables
        std::vector<std::pair<int, int>> sorted_vars(var_count.begin(), var_count.end());
        std::sort(sorted_vars.begin(), sorted_vars.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });

        int flip_count = std::min(3, (int)sorted_vars.size());
        for (int i = 0; i < flip_count; i++) {
            int var = sorted_vars[i].first;
            new_state[var] = 1 - new_state[var];
        }

        return new_state;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars,
                        int max_steps = 1000) {
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

        // Mabilis na initial local search
        bool found = local_search_fast(clauses, state, num_vars, 1000, result.flips);
        if (found) {
            result.satisfiable = true;
            result.steps = 1;
        }

        // Kung hindi pa, mag-escape
        int escape_count = 0;
        while (!result.satisfiable && result.steps < max_steps) {
            result.steps++;

            // Local search
            found = local_search_fast(clauses, state, num_vars, 500, result.flips);
            if (found) {
                result.satisfiable = true;
                break;
            }

            // Golden escape
            state = golden_escape_fast(clauses, state, num_vars, escape_count);
            result.escapes++;
            escape_count++;

            // Check kung solved na
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

            if (all_sat) {
                result.satisfiable = true;
                break;
            }

            // Kung sobrang dami ng escapes, i-restart
            if (escape_count > 10) {
                for (int v = 0; v < num_vars; v++) {
                    state[v] = (neg_count[v] > pos_count[v]) ? 0 : 1;
                }
                escape_count = 0;
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

} // namespace GoldenEscapeFast
