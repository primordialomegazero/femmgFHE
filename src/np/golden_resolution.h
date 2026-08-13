#pragma once
#include <cmath>
#include <vector>
#include <set>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <chrono>

namespace GoldenResolution {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenResolutionProver {
public:
    struct ProofResult {
        bool unsatisfiable;
        long long steps;
        int resolution_steps;
        double golden_score;
        std::vector<std::vector<int>> proof_trace;
    };

    // I-check kung may complementary literals ang dalawang clauses
    static bool has_complementary(const std::vector<int>& c1, const std::vector<int>& c2) {
        for (int lit1 : c1) {
            for (int lit2 : c2) {
                if (lit1 == -lit2) return true;
            }
        }
        return false;
    }

    // Golden resolution: pagsamahin ang dalawang clauses
    static std::vector<int> resolve(const std::vector<int>& c1, const std::vector<int>& c2) {
        std::set<int> resolved;
        bool has_contradiction = false;

        for (int lit : c1) {
            if (std::find(c2.begin(), c2.end(), -lit) != c2.end()) {
                has_contradiction = true;
            } else {
                resolved.insert(lit);
            }
        }

        for (int lit : c2) {
            if (std::find(c1.begin(), c1.end(), -lit) == c1.end()) {
                resolved.insert(lit);
            }
        }

        return std::vector<int>(resolved.begin(), resolved.end());
    }

    // Golden resolution proof para sa unsat
    static ProofResult prove_unsat(const std::vector<std::vector<int>>& clauses) {
        ProofResult result;
        result.unsatisfiable = false;
        result.steps = 0;
        result.resolution_steps = 0;
        result.golden_score = 0.0;

        std::vector<std::vector<int>> current = clauses;
        std::set<std::vector<int>> seen;

        for (int iter = 0; iter < 100000; iter++) {
            result.steps = iter;
            bool found_new = false;

            for (size_t i = 0; i < current.size(); i++) {
                for (size_t j = i + 1; j < current.size(); j++) {
                    if (!has_complementary(current[i], current[j])) continue;

                    result.resolution_steps++;
                    std::vector<int> resolved = resolve(current[i], current[j]);

                    // Golden score: φ para sa non-empty, ψ para sa empty
                    if (resolved.empty()) {
                        // Empty clause = contradiction!
                        result.unsatisfiable = true;
                        result.golden_score = PHI * iter;
                        return result;
                    }

                    if (seen.find(resolved) == seen.end()) {
                        seen.insert(resolved);
                        current.push_back(resolved);
                        found_new = true;
                    }
                }
            }

            if (!found_new) {
                // Walang bagong clauses na ma-generate
                break;
            }
        }

        result.golden_score = PSI * result.steps;
        return result;
    }
};

} // namespace GoldenResolution
