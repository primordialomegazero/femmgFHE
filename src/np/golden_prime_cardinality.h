#pragma once
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>

namespace GoldenPrimeCardinality {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// Fibonacci primes para sa cardinality encoding
// F(3)=2, F(4)=3, F(5)=5, F(7)=13, F(11)=89, F(13)=233
constexpr int FIB_PRIMES[] = {2, 3, 5, 13, 89, 233, 1597};

class GoldenPrimeSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        double golden_score;
        long long steps;
    };

    // Fibonacci prime weight para sa cardinality
    static double fib_prime_weight(int var, int pos, int total) {
        int fib_idx = pos % 7;
        double base = FIB_PRIMES[fib_idx];
        // Golden ratio modulation
        double golden = (pos % 2 == 0) ? PHI : PSI;
        return base * golden;
    }

    // Golden prime solver na may cardinality constraints
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.golden_score = 0.0;
        result.steps = 0;

        std::vector<double> weights(num_vars, 0.0);

        // Para sa bawat clause, i-compute ang golden prime weight
        for (const auto& clause : clauses) {
            for (size_t pos = 0; pos < clause.size(); pos++) {
                int lit = clause[pos];
                int var = std::abs(lit) - 1;
                double w = fib_prime_weight(var, pos, clause.size());
                if (lit > 0) weights[var] += w;
                else weights[var] -= w;
            }
            result.steps++;
        }

        // Assignment: kung ang weight ay positive
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = weights[v] > 0;
        }

        // I-verify
        int satisfied = 0;
        for (const auto& clause : clauses) {
            bool clause_satisfied = false;
            for (int lit : clause) {
                int var = std::abs(lit) - 1;
                bool val = result.assignment[var];
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    clause_satisfied = true;
                    break;
                }
            }
            if (clause_satisfied) satisfied++;
            result.steps++;
        }

        result.satisfiable = (satisfied == (int)clauses.size());
        result.golden_score = (double)satisfied / clauses.size() * PHI;
        return result;
    }
};

} // namespace GoldenPrimeCardinality
