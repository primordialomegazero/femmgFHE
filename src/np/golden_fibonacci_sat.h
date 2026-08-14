#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>

namespace GoldenFibonacciSAT {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenFibonacciSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    // Fibonacci numbers
    static std::vector<long long> generate_fibonacci(int n) {
        std::vector<long long> fib(n);
        fib[0] = 1;
        if (n > 1) fib[1] = 1;
        for (int i = 2; i < n; i++) {
            fib[i] = fib[i-1] + fib[i-2];
        }
        return fib;
    }

    // Check kung satisfied ang clause
    static bool is_clause_satisfied(const std::vector<int>& clause,
                                    const std::vector<int>& state) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            bool val = state[var] == 1;
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                return true;
            }
        }
        return false;
    }

    static int count_unsat(const std::vector<std::vector<int>>& clauses,
                           const std::vector<int>& state) {
        int unsat = 0;
        for (const auto& clause : clauses) {
            if (!is_clause_satisfied(clause, state)) {
                unsat++;
            }
        }
        return unsat;
    }

    // Fibonacci phase assignment
    static std::vector<int> fibonacci_assignment(const std::vector<std::vector<int>>& clauses,
                                                 int num_vars) {
        auto fib = generate_fibonacci(num_vars);
        std::vector<int> state(num_vars);

        // I-assign batay sa Fibonacci parity pattern
        // F(0)=1(odd), F(1)=1(odd), F(2)=2(even), F(3)=3(odd), F(4)=5(odd), F(5)=8(even)...
        for (int v = 0; v < num_vars; v++) {
            state[v] = (fib[v] % 2 == 0) ? -1 : 1;
        }

        return state;
    }

    // Fibonacci phase shift: i-rotate ang assignment
    static std::vector<int> fibonacci_shift(const std::vector<int>& base_state,
                                            int shift) {
        std::vector<int> shifted = base_state;
        int n = shifted.size();
        
        for (int i = 0; i < n; i++) {
            shifted[i] = base_state[(i + shift) % n];
        }
        
        return shifted;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Base Fibonacci assignment
        std::vector<int> base_state = fibonacci_assignment(clauses, num_vars);

        // I-try ang iba't ibang phase shifts
        for (int shift = 0; shift < num_vars && !result.satisfiable; shift++) {
            result.steps = shift + 1;
            
            std::vector<int> state = fibonacci_shift(base_state, shift);
            int unsat = count_unsat(clauses, state);

            if (unsat == 0) {
                result.satisfiable = true;
                result.assignment.resize(num_vars);
                for (int v = 0; v < num_vars; v++) {
                    result.assignment[v] = (state[v] == 1);
                }
                break;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenFibonacciSAT
