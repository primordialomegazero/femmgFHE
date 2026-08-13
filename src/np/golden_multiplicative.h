#pragma once
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>

namespace GoldenMultiplicative {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenMultiplicativeSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        double golden_score;
        long long steps;
    };

    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.golden_score = 0.0;
        result.steps = 0;

        std::vector<bool> assignment(num_vars, true);

        for (int iter = 0; iter < num_vars * 10; iter++) {
            result.steps = iter;

            int worst_var = -1;
            double worst_score = -1e18;

            for (int v = 0; v < num_vars; v++) {
                assignment[v] = !assignment[v];
                double score = evaluate(clauses, assignment);
                assignment[v] = !assignment[v];

                if (score > worst_score) {
                    worst_score = score;
                    worst_var = v;
                }
            }

            if (worst_var == -1) break;

            assignment[worst_var] = !assignment[worst_var];

            bool all_satisfied = true;
            for (const auto& clause : clauses) {
                bool clause_satisfied = false;
                for (int lit : clause) {
                    int var = std::abs(lit) - 1;
                    bool val = assignment[var];
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        clause_satisfied = true;
                        break;
                    }
                }
                if (!clause_satisfied) {
                    all_satisfied = false;
                    break;
                }
            }

            if (all_satisfied) {
                result.satisfiable = true;
                result.assignment = assignment;
                result.golden_score = evaluate(clauses, assignment);
                return result;
            }
        }

        result.assignment = assignment;
        return result;
    }

private:
    static double evaluate(const std::vector<std::vector<int>>& clauses,
                           const std::vector<bool>& assignment) {
        double score = 0.0;
        for (const auto& clause : clauses) {
            bool clause_satisfied = false;
            for (int lit : clause) {
                int var = std::abs(lit) - 1;
                bool val = assignment[var];
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    clause_satisfied = true;
                    break;
                }
            }
            score += clause_satisfied ? PHI : PSI;
        }
        return score;
    }
};

} // namespace GoldenMultiplicative
