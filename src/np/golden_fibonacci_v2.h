#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>

namespace GoldenFibonacciV2 {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenFibonacciV2Solver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long fibonacci_assignments;
        double time_ms;
    };

private:
    // Generate Fibonacci numbers
    static std::vector<long long> generate_fibonacci(int n) {
        std::vector<long long> fib(n);
        fib[0] = 1;
        if (n > 1) fib[1] = 1;
        for (int i = 2; i < n; i++) {
            fib[i] = fib[i-1] + fib[i-2];
        }
        return fib;
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

    // Fibonacci-based assignment
    static std::vector<int> fibonacci_assignment(const std::vector<std::vector<int>>& clauses,
                                                 int num_vars) {
        auto fib = generate_fibonacci(num_vars);
        
        // Fibonacci parity: F(0)=1(odd), F(1)=1(odd), F(2)=2(even), F(3)=3(odd)...
        // Pattern: ODD, ODD, EVEN, ODD, ODD, EVEN...
        std::vector<int> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            state[v] = (fib[v] % 2 == 0) ? -1 : 1;
        }
        
        return state;
    }

    // Fibonacci golden ratio assignment: gamitin ang ratio ng consecutive Fibonacci numbers
    static std::vector<int> fibonacci_golden_assignment(const std::vector<std::vector<int>>& clauses,
                                                        int num_vars) {
        auto fib = generate_fibonacci(num_vars + 1);
        std::vector<int> state(num_vars);
        
        for (int v = 0; v < num_vars; v++) {
            // F(v+1)/F(v) → φ
            double ratio = (double)fib[v+1] / fib[v];
            
            // Kung ratio ay malapit sa φ (1.618), TRUE
            // Kung malapit sa ψ (-0.618), FALSE
            if (ratio > 1.0) {
                state[v] = 1;
            } else {
                state[v] = -1;
            }
        }
        
        return state;
    }

    // Fibonacci spiral assignment: mag-ikot sa variables
    static std::vector<int> fibonacci_spiral_assignment(const std::vector<std::vector<int>>& clauses,
                                                        int num_vars) {
        std::vector<int> state(num_vars, -1);  // Default FALSE
        
        // Ang Fibonacci spiral ay sumasaklaw sa lahat ng numbers
        // Gamitin ang Fibonacci numbers bilang indices
        auto fib = generate_fibonacci(num_vars);
        
        for (long long f : fib) {
            int idx = f % num_vars;
            state[idx] = 1;  // Ang mga Fibonacci positions ay TRUE
        }
        
        return state;
    }

    // Local search na may Fibonacci guidance
    static bool fibonacci_local_search(const std::vector<std::vector<int>>& clauses,
                                       std::vector<int>& state,
                                       int num_vars,
                                       std::vector<int>& unsat_list,
                                       int max_flips) {
        auto fib = generate_fibonacci(std::min(num_vars, 100));
        
        int current_unsat = count_unsat(clauses, state, &unsat_list);
        if (current_unsat == 0) return true;

        for (int flip = 0; flip < max_flips; flip++) {
            if (unsat_list.empty()) {
                current_unsat = count_unsat(clauses, state, &unsat_list);
                if (current_unsat == 0) return true;
            }

            // Pumili ng variable gamit ang Fibonacci index
            int fib_idx = fib[flip % fib.size()] % num_vars;
            
            // I-flip ang variable na ito
            state[fib_idx] = 1 - state[fib_idx];
            
            // Check kung nakatulong
            int new_unsat = count_unsat(clauses, state, &unsat_list);
            
            if (new_unsat < current_unsat) {
                current_unsat = new_unsat;
            } else if (new_unsat > current_unsat) {
                // I-revert kung lumala
                state[fib_idx] = 1 - state[fib_idx];
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
        all_states.push_back(fibonacci_assignment(clauses, num_vars));
        all_states.push_back(fibonacci_golden_assignment(clauses, num_vars));
        all_states.push_back(fibonacci_spiral_assignment(clauses, num_vars));

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
            bool found = fibonacci_local_search(clauses, test_state, num_vars, test_unsat_list, 500);
            
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
            auto fib = generate_fibonacci(std::min(num_vars, 50));
            
            for (int f : fib) {
                int idx = f % num_vars;
                combined[idx] = 1 - combined[idx];
            }
            
            int unsat = count_unsat(clauses, combined, &unsat_list);
            if (unsat == 0) {
                result.satisfiable = true;
                best_state = combined;
            } else {
                // Fibonacci local search sa combined
                std::vector<int> test_state = combined;
                std::vector<int> test_unsat_list = unsat_list;
                bool found = fibonacci_local_search(clauses, test_state, num_vars, test_unsat_list, 1000);
                
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

} // namespace GoldenFibonacciV2
