#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <random>

namespace GoldenHybridCollapse {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenHybridSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long propagations;
        long long local_flips;
        double time_ms;
    };

private:
    // Entanglement-based initial assignment (mabilis pero maaring may mali)
    static std::vector<int> entanglement_initialize(
        const std::vector<std::vector<int>>& clauses,
        int num_vars) {
        
        std::vector<int> state(num_vars, 0);
        std::vector<int> pos_count(num_vars, 0);
        std::vector<int> neg_count(num_vars, 0);

        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (lit > 0) pos_count[var]++;
                else neg_count[var]++;
            }
        }

        // Golden ratio initialization: mas maraming negative occurrences → FALSE
        for (int v = 0; v < num_vars; v++) {
            state[v] = (neg_count[v] > pos_count[v]) ? -1 : 1;
        }

        return state;
    }

    // Mabilis na local search na may incremental counting
    static int count_unsat_fast(const std::vector<std::vector<int>>& clauses,
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

    // Golden ratio guided local search
    static bool golden_local_search(const std::vector<std::vector<int>>& clauses,
                                    std::vector<int>& state,
                                    int num_vars,
                                    int max_flips,
                                    long long& flips) {
        int current_unsat = count_unsat_fast(clauses, state);

        if (current_unsat == 0) return true;

        std::vector<int> tabu(num_vars, 0);
        int tabu_tenure = (int)std::sqrt(num_vars) + 1;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, num_vars - 1);

        for (int flip = 0; flip < max_flips; flip++) {
            // Hanapin ang best variable na i-flip
            int best_var = -1;
            int best_unsat = current_unsat + 1;

            // Subukan ang random sampling para sa malalaking instances
            int sample_size = std::min(num_vars, 50);

            for (int s = 0; s < sample_size; s++) {
                int v = dis(gen);
                if (tabu[v] > 0) continue;

                state[v] = 1 - state[v];  // Trial flip
                int new_unsat = count_unsat_fast(clauses, state);
                state[v] = 1 - state[v];  // Revert

                if (new_unsat < best_unsat) {
                    best_unsat = new_unsat;
                    best_var = v;
                }
            }

            // Kung walang improvement, subukan ang random walk
            if (best_var == -1) {
                // Pumili ng variable mula sa unsatisfied clause
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
                        // Pumili ng random literal mula sa clause na ito
                        int lit = clause[dis(gen) % clause.size()];
                        best_var = abs(lit) - 1;
                        break;
                    }
                }

                if (best_var == -1) break;  // Wala nang unsat clauses
            }

            // Apply ang flip
            state[best_var] = 1 - state[best_var];
            current_unsat = best_unsat;
            flips++;

            // Update tabu
            for (int v = 0; v < num_vars; v++) {
                if (tabu[v] > 0) tabu[v]--;
            }
            tabu[best_var] = tabu_tenure;

            if (current_unsat == 0) return true;

            // Kung masyadong marami nang flips, break
            if (flips > max_flips * 2) break;
        }

        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars,
                        int max_steps = 100) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.propagations = 0;
        result.local_flips = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Phase 1: Entanglement initialization (mabilis)
        std::vector<int> state = entanglement_initialize(clauses, num_vars);
        result.propagations = num_vars;

        // Phase 2: Local search refinement
        bool found = golden_local_search(clauses, state, num_vars, 5000, result.local_flips);
        result.steps = 1;

        if (found) {
            result.satisfiable = true;
        } else {
            // Subukan muli na may ibang initialization
            for (int attempt = 1; attempt < max_steps && !result.satisfiable; attempt++) {
                result.steps = attempt + 1;

                // Random restart na may golden ratio influence
                for (int v = 0; v < num_vars; v++) {
                    state[v] = (v % 2 == 0) ? 1 : -1;
                }

                found = golden_local_search(clauses, state, num_vars, 5000, result.local_flips);
                if (found) {
                    result.satisfiable = true;
                    break;
                }
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

} // namespace GoldenHybridCollapse
