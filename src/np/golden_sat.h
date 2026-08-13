#pragma once
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>

namespace GoldenSAT {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenSATSolver {
public:
    struct SATResult {
        bool satisfiable;
        std::vector<bool> assignment;
        double golden_score;
        long long steps;
    };

    static double golden_score(const std::vector<std::vector<int>>& clauses,
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
            if (clause_satisfied) {
                score += PHI;
            } else {
                score += PSI;
            }
        }
        return score;
    }

    static SATResult solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        SATResult result;
        result.satisfiable = false;
        result.golden_score = 0.0;
        result.steps = 0;

        // Golden phase transition: maraming restarts para sa malalaking instances
        int num_restarts = 50 + num_vars * 10;
        uint64_t base_seed = 42;

        for (int restart = 0; restart < num_restarts; restart++) {
            std::vector<bool> assignment(num_vars);
            uint64_t seed = base_seed + restart * 1000000;
            for (int v = 0; v < num_vars; v++) {
                seed ^= (seed << 13);
                seed ^= (seed >> 7);
                seed ^= (seed << 17);
                assignment[v] = seed & 1;
            }

            double best_score = golden_score(clauses, assignment);

            for (int iter = 0; iter < 1000; iter++) {
                result.steps++;

                int best_flip = -1;
                double best_new_score = best_score;

                for (int v = 0; v < num_vars; v++) {
                    assignment[v] = !assignment[v];
                    double new_score = golden_score(clauses, assignment);
                    assignment[v] = !assignment[v];

                    if (new_score > best_new_score) {
                        best_new_score = new_score;
                        best_flip = v;
                    }
                }

                if (best_flip == -1) break;

                assignment[best_flip] = !assignment[best_flip];
                best_score = best_new_score;

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
                    result.golden_score = best_score;
                    result.assignment = assignment;
                    return result;
                }
            }
        }

        return result;
    }
};

} // namespace GoldenSAT
