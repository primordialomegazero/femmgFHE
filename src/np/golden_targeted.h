#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <random>

namespace GoldenTargeted {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenTargetedSolver {
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
                           const std::vector<int>& state,
                           std::vector<int>* unsat_clauses = nullptr) {
        int unsat = 0;
        if (unsat_clauses) unsat_clauses->clear();
        
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
                if (unsat_clauses) unsat_clauses->push_back(ci);
            }
        }
        return unsat;
    }

    // Targeted local search na may clause-aware flipping
    static bool targeted_local_search(const std::vector<std::vector<int>>& clauses,
                                      std::vector<int>& state,
                                      int num_vars,
                                      int max_flips,
                                      long long& flip_count) {
        std::vector<int> unsat_clauses;
        int current_unsat = count_unsat(clauses, state, &unsat_clauses);

        if (current_unsat == 0) return true;

        std::random_device rd;
        std::mt19937 gen(42);
        std::uniform_int_distribution<> dis(0, num_vars - 1);

        for (int flip = 0; flip < max_flips; flip++) {
            if (unsat_clauses.empty()) {
                current_unsat = count_unsat(clauses, state, &unsat_clauses);
                if (current_unsat == 0) return true;
            }

            // Pumili ng unsat clause na may PINAKAMALIIT na variables
            int best_clause_idx = unsat_clauses[0];
            int min_false_count = 999;

            for (int ci : unsat_clauses) {
                int false_count = 0;
                for (int lit : clauses[ci]) {
                    int var = abs(lit) - 1;
                    bool val = state[var] == 1;
                    if (!((lit > 0 && val) || (lit < 0 && !val))) {
                        false_count++;
                    }
                }
                if (false_count < min_false_count) {
                    min_false_count = false_count;
                    best_clause_idx = ci;
                }
            }

            // Flip ang variable na magpapa-satisfy sa clause
            // Pumili ng literal na may pinakamalaking impact
            int best_var = -1;
            int best_improvement = -1;

            for (int lit : clauses[best_clause_idx]) {
                int var = abs(lit) - 1;
                
                // Trial flip
                state[var] = 1 - state[var];
                int new_unsat = count_unsat(clauses, state);
                state[var] = 1 - state[var];  // Revert

                int improvement = current_unsat - new_unsat;
                if (improvement > best_improvement) {
                    best_improvement = improvement;
                    best_var = var;
                }
            }

            if (best_var == -1) {
                // Random fallback
                int ci = unsat_clauses[gen() % unsat_clauses.size()];
                int lit = clauses[ci][gen() % clauses[ci].size()];
                best_var = abs(lit) - 1;
            }

            // Flip ang variable
            state[best_var] = 1 - state[best_var];
            flip_count++;

            // I-update
            current_unsat = count_unsat(clauses, state, &unsat_clauses);

            if (current_unsat == 0) return true;

            // Kung stuck sa 1, subukan ang random walk
            if (current_unsat <= 2 && flip > 50) {
                for (int escape = 0; escape < 20; escape++) {
                    int v = dis(gen);
                    state[v] = 1 - state[v];
                    flip_count++;
                    
                    int new_unsat = count_unsat(clauses, state, &unsat_clauses);
                    if (new_unsat == 0) return true;
                    if (new_unsat < current_unsat) {
                        current_unsat = new_unsat;
                        break;
                    }
                }
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

        // Targeted local search
        long long flips = 0;
        bool found = targeted_local_search(clauses, state, num_vars, 2000, flips);

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

} // namespace GoldenTargeted
