#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>

namespace GoldenFibFusion {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenFibFusionSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long fib_steps;
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

    // Fibonacci-Golden assignment: gamitin ang ratio ng consecutive Fib numbers
    static std::vector<int> fib_golden_assignment(int num_vars) {
        std::vector<int> state(num_vars);
        
        // Hindi na natin kailangan ang actual Fibonacci numbers
        // Ang ratio F(n+1)/F(n) ay nagco-converge sa φ ≈ 1.618
        // Pattern: 1, 2, 1.5, 1.667, 1.6, 1.625, 1.615, 1.619, ...
        
        for (int v = 0; v < num_vars; v++) {
            // Gamitin ang golden angle para sa spiral pattern
            double golden_angle = 2.399963229728653;  // 2π/φ²
            double value = std::sin(v * golden_angle);
            
            if (value > 0) {
                state[v] = 1;
            } else if (value < 0) {
                state[v] = -1;
            } else {
                state[v] = (v % 2 == 0) ? 1 : -1;
            }
        }
        
        return state;
    }

    // Fibonacci search: gamitin ang Fibonacci numbers bilang jump sizes
    static bool fib_golden_search(const std::vector<std::vector<int>>& clauses,
                                  std::vector<int>& state,
                                  int num_vars,
                                  std::vector<int>& unsat_list,
                                  int max_steps,
                                  long long& fib_steps) {
        int current_unsat = count_unsat(clauses, state, &unsat_list);
        if (current_unsat == 0) return true;

        // Fibonacci-like jump sizes (modulo para hindi mag-overflow)
        std::vector<int> jumps = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144};
        
        std::random_device rd;
        std::mt19937 gen(42);
        std::uniform_int_distribution<> dis(0, num_vars - 1);

        int fib_index = 0;
        for (int step = 0; step < max_steps; step++) {
            if (unsat_list.empty()) {
                current_unsat = count_unsat(clauses, state, &unsat_list);
                if (current_unsat == 0) return true;
            }

            // Fibonacci jump: pumili ng variable na may Fibonacci distance
            int jump = jumps[fib_index % jumps.size()];
            fib_index++;
            
            // Hanapin ang variable na may pinakamalaking impact
            std::unordered_map<int, int> impact;
            for (int ci : unsat_list) {
                for (int lit : clauses[ci]) {
                    impact[abs(lit) - 1]++;
                }
            }

            if (impact.empty()) break;

            // Pumili ng variable na may pinakamalaking impact at Fibonacci jump
            int best_var = -1;
            int best_score = 0;
            
            for (const auto& [var, count] : impact) {
                // Fibonacci score: impact * φ^jump
                double fib_score = count * std::pow(PHI, jump % 5);
                if (fib_score > best_score) {
                    best_score = (int)fib_score;
                    best_var = var;
                }
            }

            if (best_var == -1) {
                // Random from unsat clause
                int ci = unsat_list[gen() % unsat_list.size()];
                int lit = clauses[ci][gen() % clauses[ci].size()];
                best_var = abs(lit) - 1;
            }

            // I-flip ang variable
            state[best_var] = 1 - state[best_var];
            fib_steps++;
            
            current_unsat = count_unsat(clauses, state, &unsat_list);
            
            if (current_unsat == 0) return true;

            // Fibonacci restart: kung stuck, mag-jump ng malaki
            if (step > 50 && current_unsat > clauses.size() * 0.05) {
                // I-flip ang maraming variables sabay-sabay (Fibonacci pattern)
                int restart_jump = jumps[(fib_index + 3) % jumps.size()];
                for (int i = 0; i < restart_jump; i++) {
                    int v = (best_var + i * jump) % num_vars;
                    state[v] = 1 - state[v];
                    fib_steps++;
                }
                current_unsat = count_unsat(clauses, state, &unsat_list);
                
                if (current_unsat == 0) return true;
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
        result.fib_steps = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Fusion: Fib-Golden assignment
        std::vector<int> state = fib_golden_assignment(num_vars);
        result.steps = 1;

        std::vector<int> unsat_list;
        int current_unsat = count_unsat(clauses, state, &unsat_list);

        if (current_unsat == 0) {
            result.satisfiable = true;
        } else {
            // Fib-Golden search
            bool found = fib_golden_search(clauses, state, num_vars, unsat_list, 500, result.fib_steps);
            result.steps = 2;

            if (found) {
                result.satisfiable = true;
            } else {
                // Alternative: reverse Fib-Golden
                for (int v = 0; v < num_vars; v++) {
                    state[v] = 1 - state[v];
                }
                
                found = fib_golden_search(clauses, state, num_vars, unsat_list, 500, result.fib_steps);
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

} // namespace GoldenFibFusion
