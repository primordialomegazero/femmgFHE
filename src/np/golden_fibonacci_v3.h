#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>

namespace GoldenFibonacciV3 {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenFibonacciV3Solver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long fibonacci_assignments;
        double time_ms;
    };

private:
    // Fibonacci pattern na walang overflow
    // Pattern: 1,1,0,1,1,0,1,1,0... (odd, odd, even)
    static bool fib_parity(int n) {
        // F(n) is even if n % 3 == 2
        return (n % 3 != 2);
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

    // Fibonacci parity assignment
    static std::vector<int> fibonacci_parity_assignment(int num_vars) {
        std::vector<int> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            state[v] = fib_parity(v) ? 1 : -1;
        }
        return state;
    }

    // Fibonacci golden ratio pattern: F(n+1)/F(n) → φ
    // Pattern: ang ratio ay > 1 para sa lahat (kasi φ > 1)
    static std::vector<int> fibonacci_ratio_assignment(int num_vars) {
        std::vector<int> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            // Fibonacci numbers modulo 3: 1,1,2,0,2,2,1,0,1,1,2,0...
            int mod3 = v % 4;
            if (mod3 == 0 || mod3 == 1) state[v] = 1;
            else if (mod3 == 2) state[v] = -1;
            else state[v] = 1;
        }
        return state;
    }

    // Fibonacci spiral: gamitin ang golden angle (137.5 degrees)
    static std::vector<int> fibonacci_spiral_assignment(int num_vars) {
        std::vector<int> state(num_vars, -1);
        
        // Golden angle: 2π/φ² ≈ 2.4 radians
        // I-map sa variables
        double golden_angle = 2.399963229728653;
        
        for (int i = 0; i < num_vars; i++) {
            double angle = i * golden_angle;
            int idx = (int)(std::abs(std::sin(angle)) * num_vars) % num_vars;
            state[idx] = 1;
        }
        
        return state;
    }

    // Local search na may Fibonacci pattern guidance
    static bool fibonacci_pattern_search(const std::vector<std::vector<int>>& clauses,
                                         std::vector<int>& state,
                                         int num_vars,
                                         std::vector<int>& unsat_list,
                                         int max_flips) {
        int current_unsat = count_unsat(clauses, state, &unsat_list);
        if (current_unsat == 0) return true;

        std::random_device rd;
        std::mt19937 gen(42);
        std::uniform_int_distribution<> dis(0, num_vars - 1);

        for (int flip = 0; flip < max_flips; flip++) {
            if (unsat_list.empty()) {
                current_unsat = count_unsat(clauses, state, &unsat_list);
                if (current_unsat == 0) return true;
            }

            // Fibonacci pattern: pumili ng variable na may Fibonacci parity
            int fib_idx = flip % 3;
            int var;
            
            if (fib_idx == 2) {
                // F(2)=2 ay even, pumili ng random from unsat
                int ci = unsat_list[gen() % unsat_list.size()];
                int lit = clauses[ci][gen() % clauses[ci].size()];
                var = abs(lit) - 1;
            } else {
                // F(0)=1, F(1)=1 ay odd, pumili ng random
                var = dis(gen);
            }

            // Trial flip
            state[var] = 1 - state[var];
            int new_unsat = count_unsat(clauses, state, &unsat_list);

            if (new_unsat < current_unsat) {
                current_unsat = new_unsat;
            } else {
                // I-revert kung hindi nakatulong
                state[var] = 1 - state[var];
                count_unsat(clauses, state, &unsat_list);
            }

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
        result.fibonacci_assignments = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Subukan ang iba't ibang Fibonacci-based assignments
        std::vector<std::vector<int>> all_states;
        all_states.push_back(fibonacci_parity_assignment(num_vars));
        all_states.push_back(fibonacci_ratio_assignment(num_vars));
        all_states.push_back(fibonacci_spiral_assignment(num_vars));

        std::vector<int> unsat_list;
        int best_unsat = num_vars + 1;
        std::vector<int> best_state;

        for (auto& state : all_states) {
            int unsat = count_unsat(clauses, state, &unsat_list);
            result.fibonacci_assignments++;

            if (unsat == 0) {
                result.satisfiable = true;
                best_state = state;
                break;
            }

            if (unsat < best_unsat) {
                best_unsat = unsat;
                best_state = state;
            }

            // Local search para sa bawat assignment
            std::vector<int> test_state = state;
            std::vector<int> test_unsat_list = unsat_list;
            bool found = fibonacci_pattern_search(clauses, test_state, num_vars, test_unsat_list, 500);
            
            if (found) {
                result.satisfiable = true;
                best_state = test_state;
                break;
            }
        }

        // Kung wala pang nakita, subukan ang combined approach
        if (!result.satisfiable && !best_state.empty()) {
            result.steps = 1;
            
            // I-combine ang best state sa Fibonacci pattern
            std::vector<int> combined = best_state;
            for (int v = 0; v < num_vars; v++) {
                if (fib_parity(v)) {
                    combined[v] = 1 - combined[v];  // I-flip ang odd positions
                }
            }
            
            int unsat = count_unsat(clauses, combined, &unsat_list);
            if (unsat == 0) {
                result.satisfiable = true;
                best_state = combined;
            } else {
                // Fibonacci local search sa combined
                std::vector<int> test_state = combined;
                std::vector<int> test_unsat_list = unsat_list;
                bool found = fibonacci_pattern_search(clauses, test_state, num_vars, test_unsat_list, 1000);
                
                if (found) {
                    result.satisfiable = true;
                    best_state = test_state;
                }
            }
        }

        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            if (best_state.empty()) {
                result.assignment[v] = false;
            } else {
                result.assignment[v] = (best_state[v] == 1);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenFibonacciV3
