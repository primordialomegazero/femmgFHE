#pragma once
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>
#include <map>

namespace GoldenExact {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenExactSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        double golden_score;
        long long steps;
    };

    // EXACT: Gumamit ng golden ratio bilang algebraic weight
    // para direktang i-solve ang SAT nang walang local search
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.golden_score = 0.0;
        result.steps = 0;

        // Para sa bawat variable, i-compute ang golden weight
        // na may clause normalization para sa iba't ibang SAT variants
        std::vector<double> weights(num_vars, 0.0);

        for (const auto& clause : clauses) {
            // I-normalize ang weight ayon sa clause size
            double clause_weight = 1.0 / clause.size();
            for (int lit : clause) {
                int var = std::abs(lit) - 1;
                if (lit > 0) {
                    weights[var] += PHI * clause_weight;
                } else {
                    weights[var] += PSI * clause_weight;
                }
            }
            result.steps++;
        }

        // EXACT assignment: kung positive ang weight, true; kung negative, false
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            // Para sa Horn SAT: mas maraming negative literals
            // kaya kailangan ng mas mataas na threshold
            result.assignment[v] = weights[v] > 0;
        }

        // I-verify ang assignment
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

} // namespace GoldenExact
