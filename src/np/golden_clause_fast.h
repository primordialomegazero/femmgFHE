#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <random>

namespace GoldenClauseFast {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenClauseFastSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double signature_value;
        double time_ms;
    };

private:
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

    // Mabilis na local search na may watched variables
    static bool fast_local_search(const std::vector<std::vector<int>>& clauses,
                                  std::vector<int>& state,
                                  int num_vars,
                                  int max_flips,
                                  long long& flip_count) {
        int current_unsat = count_unsat(clauses, state);
        if (current_unsat == 0) return true;

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

        std::random_device rd;
        std::mt19937 gen(42);
        std::uniform_int_distribution<> dis(0, num_vars - 1);

        for (int flip = 0; flip < max_flips; flip++) {
            // Pumili ng variable mula sa unsatisfied clauses (mas targeted)
            if (unsat_clauses.empty()) {
                current_unsat = count_unsat(clauses, state);
                if (current_unsat == 0) return true;
                
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
                if (unsat_clauses.empty()) return true;
            }

            // Pumili ng random unsat clause at random literal
            int ci = unsat_clauses[gen() % unsat_clauses.size()];
            int lit = clauses[ci][gen() % clauses[ci].size()];
            int var = abs(lit) - 1;

            // I-flip at i-check
            state[var] = 1 - state[var];
            flip_count++;

            // I-update ang unsat clauses
            unsat_clauses.clear();
            current_unsat = 0;
            for (int cj = 0; cj < clauses.size(); cj++) {
                bool sat = false;
                for (int l : clauses[cj]) {
                    int v = abs(l) - 1;
                    bool val = state[v] == 1;
                    if ((l > 0 && val) || (l < 0 && !val)) {
                        sat = true;
                        break;
                    }
                }
                if (!sat) {
                    unsat_clauses.push_back(cj);
                    current_unsat++;
                }
            }

            if (current_unsat == 0) return true;

            // Early termination
            if (flip > 200 && current_unsat > clauses.size() * 0.01) {
                return false;
            }
        }

        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars) {
        Result result;
        result.steps = 1;
        result.signature_value = 0.0;

        auto start = std::chrono::high_resolution_clock::now();

        // Quick signature computation
        int all_pos = 0, all_neg = 0, mixed = 0;
        for (const auto& clause : clauses) {
            int pos_count = 0, neg_count = 0;
            for (int lit : clause) {
                if (lit > 0) pos_count++;
                else neg_count++;
            }
            if (pos_count == 3) all_pos++;
            else if (neg_count == 3) all_neg++;
            else mixed++;
        }

        int total = clauses.size();
        result.signature_value = std::abs((double)all_pos / total - (double)all_neg / total) +
                                 std::abs((double)mixed / total - 0.618);

        // Direct assignment
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

        // Mabilis na local search
        long long flips = 0;
        bool found = fast_local_search(clauses, state, num_vars, 1000, flips);

        result.satisfiable = found;
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] == 1);
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenClauseFast
