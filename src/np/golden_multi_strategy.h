#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>

namespace GoldenMultiStrategy {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenMultiStrategySolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long strategy_switches;
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

    // Strategy 1: Greedy impact flip
    static bool strategy_greedy(const std::vector<std::vector<int>>& clauses,
                                std::vector<int>& state,
                                int num_vars,
                                std::vector<int>& unsat_list,
                                int max_flips) {
        int current_unsat = count_unsat(clauses, state, &unsat_list);
        if (current_unsat == 0) return true;

        for (int flip = 0; flip < max_flips; flip++) {
            if (unsat_list.empty()) {
                current_unsat = count_unsat(clauses, state, &unsat_list);
                if (current_unsat == 0) return true;
            }

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
        }

        return false;
    }

    // Strategy 2: Random walk
    static bool strategy_random_walk(const std::vector<std::vector<int>>& clauses,
                                     std::vector<int>& state,
                                     int num_vars,
                                     std::vector<int>& unsat_list,
                                     int max_flips) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, num_vars - 1);

        int current_unsat = count_unsat(clauses, state, &unsat_list);
        if (current_unsat == 0) return true;

        for (int flip = 0; flip < max_flips; flip++) {
            if (unsat_list.empty()) {
                current_unsat = count_unsat(clauses, state, &unsat_list);
                if (current_unsat == 0) return true;
            }

            // Pumili ng random variable mula sa unsat clause
            int ci = unsat_list[gen() % unsat_list.size()];
            int lit = clauses[ci][gen() % clauses[ci].size()];
            int var = abs(lit) - 1;

            state[var] = 1 - state[var];
            current_unsat = count_unsat(clauses, state, &unsat_list);

            if (current_unsat == 0) return true;
        }

        return false;
    }

    // Strategy 3: Golden ratio pattern flip
    static bool strategy_golden_pattern(const std::vector<std::vector<int>>& clauses,
                                        std::vector<int>& state,
                                        int num_vars,
                                        std::vector<int>& unsat_list,
                                        int max_flips) {
        int current_unsat = count_unsat(clauses, state, &unsat_list);
        if (current_unsat == 0) return true;

        // Fibonacci sequence para sa flip pattern
        std::vector<int> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89};

        for (int flip = 0; flip < max_flips; flip++) {
            if (unsat_list.empty()) {
                current_unsat = count_unsat(clauses, state, &unsat_list);
                if (current_unsat == 0) return true;
            }

            // I-flip ang variables na may Fibonacci index
            int fib_idx = fib[flip % fib.size()] % num_vars;
            state[fib_idx] = 1 - state[fib_idx];

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
        result.strategy_switches = 0;

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
        } else {
            // Subukan ang bawat strategy
            int max_flips = std::min(500, num_vars * 2);
            
            for (int attempt = 0; attempt < 10 && !result.satisfiable; attempt++) {
                result.steps = attempt + 1;
                result.strategy_switches++;

                std::vector<int> test_state = state;
                std::vector<int> test_unsat_list = unsat_list;
                bool found = false;

                switch (attempt % 3) {
                    case 0:
                        found = strategy_greedy(clauses, test_state, num_vars, test_unsat_list, max_flips);
                        break;
                    case 1:
                        found = strategy_random_walk(clauses, test_state, num_vars, test_unsat_list, max_flips);
                        break;
                    case 2:
                        found = strategy_golden_pattern(clauses, test_state, num_vars, test_unsat_list, max_flips);
                        break;
                }

                if (found) {
                    result.satisfiable = true;
                    state = test_state;
                    break;
                }

                // Kung walang strategy ang gumana, i-shuffle ang state
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, num_vars - 1);
                for (int shuffle = 0; shuffle < num_vars / 2; shuffle++) {
                    int v = dis(gen);
                    state[v] = 1 - state[v];
                }
                count_unsat(clauses, state, &unsat_list);
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

} // namespace GoldenMultiStrategy
