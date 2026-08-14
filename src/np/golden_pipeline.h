#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <random>

namespace GoldenPipeline {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenPipelineSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long phase1_entanglements;
        long long phase2_collapses;
        long long phase3_flips;
        double time_ms;
    };

private:
    // ===== PHASE 1: QUANTUM ENTANGLEMENT INITIALIZATION =====
    // Mabilis na pag-assign ng initial values gamit ang entanglement structure
    static void phase1_entanglement_init(
        const std::vector<std::vector<int>>& clauses,
        std::vector<int>& state,
        int num_vars,
        long long& entanglement_count) {
        
        std::vector<int> pos_count(num_vars, 0);
        std::vector<int> neg_count(num_vars, 0);
        std::vector<int> clause_count(num_vars, 0);

        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = abs(lit) - 1;
                clause_count[var]++;
                if (lit > 0) pos_count[var]++;
                else neg_count[var]++;
            }
        }

        // Golden ratio initialization
        for (int v = 0; v < num_vars; v++) {
            double ratio = (double)neg_count[v] / (double)clause_count[v];
            // φ = 0.618... kung malapit sa golden ratio, may balance
            if (std::abs(ratio - 0.618) < 0.1) {
                // Balanced: gamitin ang alternating pattern
                state[v] = (v % 2 == 0) ? 1 : -1;
            } else if (ratio > 0.618) {
                // Mas maraming negative: FALSE
                state[v] = -1;
            } else {
                // Mas maraming positive: TRUE
                state[v] = 1;
            }
            entanglement_count++;
        }
    }

    // ===== PHASE 2: BLACK HOLE COLLAPSE =====
    // I-collapse ang unsatisfied clauses papunta sa satisfaction
    static void phase2_black_hole_collapse(
        const std::vector<std::vector<int>>& clauses,
        std::vector<int>& state,
        int num_vars,
        long long& collapse_count) {
        
        // Hanapin ang unsatisfied clauses at i-absorb ang "noise"
        for (int iteration = 0; iteration < 10; iteration++) {
            bool changed = false;

            for (const auto& clause : clauses) {
                bool sat = false;
                std::vector<int> false_literals;

                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    bool val = state[var] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        sat = true;
                        break;
                    } else {
                        false_literals.push_back(lit);
                    }
                }

                if (!sat && !false_literals.empty()) {
                    // I-flip ang isang false literal para ma-satisfy
                    int lit = false_literals[0];
                    int var = abs(lit) - 1;
                    state[var] = 1 - state[var];
                    collapse_count++;
                    changed = true;
                    break;  // Isa muna, tapos re-evaluate
                }
            }

            if (!changed) break;
        }
    }

    // ===== PHASE 3: GOLDEN LOCAL SEARCH =====
    // Fine-tuning na may tabu search
    static bool phase3_local_search(
        const std::vector<std::vector<int>>& clauses,
        std::vector<int>& state,
        int num_vars,
        int max_flips,
        long long& flip_count) {
        
        auto count_unsat = [&]() {
            int unsat = 0;
            for (const auto& clause : clauses) {
                bool sat = false;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    bool val = state[var] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        sat = true;
                        break;
                    }
                }
                if (!sat) unsat++;
            }
            return unsat;
        };

        int current_unsat = count_unsat();
        if (current_unsat == 0) return true;

        std::vector<int> tabu(num_vars, 0);
        int tabu_tenure = (int)std::sqrt(num_vars) + 1;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, num_vars - 1);

        for (int flip = 0; flip < max_flips; flip++) {
            // Random sampling para sa speed
            int sample_size = std::min(num_vars, 30);
            int best_var = -1;
            int best_unsat = current_unsat + 1;

            for (int s = 0; s < sample_size; s++) {
                int v = dis(gen);
                if (tabu[v] > 0) continue;

                state[v] = 1 - state[v];
                int new_unsat = count_unsat();
                state[v] = 1 - state[v];

                if (new_unsat < best_unsat) {
                    best_unsat = new_unsat;
                    best_var = v;
                }
            }

            if (best_var == -1) {
                // Random walk mula sa unsat clause
                bool found = false;
                for (const auto& clause : clauses) {
                    bool sat = false;
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        bool val = state[var] == 1;
                        if ((lit > 0 && val) || (lit < 0 && !val)) {
                            sat = true;
                            break;
                        }
                    }
                    if (!sat) {
                        int lit = clause[dis(gen) % clause.size()];
                        best_var = abs(lit) - 1;
                        found = true;
                        break;
                    }
                }
                if (!found) break;
            }

            state[best_var] = 1 - state[best_var];
            current_unsat = best_unsat;
            flip_count++;

            for (int v = 0; v < num_vars; v++) {
                if (tabu[v] > 0) tabu[v]--;
            }
            tabu[best_var] = tabu_tenure;

            if (current_unsat == 0) return true;
        }

        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars,
                        int max_attempts = 50) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.phase1_entanglements = 0;
        result.phase2_collapses = 0;
        result.phase3_flips = 0;

        auto start = std::chrono::high_resolution_clock::now();

        for (int attempt = 0; attempt < max_attempts && !result.satisfiable; attempt++) {
            result.steps = attempt + 1;

            // Phase 1: Entanglement init
            std::vector<int> state(num_vars, 0);
            phase1_entanglement_init(clauses, state, num_vars, result.phase1_entanglements);

            // Phase 2: Black hole collapse
            phase2_black_hole_collapse(clauses, state, num_vars, result.phase2_collapses);

            // Phase 3: Local search
            bool found = phase3_local_search(clauses, state, num_vars, 3000, result.phase3_flips);
            
            if (found) {
                result.satisfiable = true;
                result.assignment.resize(num_vars);
                for (int v = 0; v < num_vars; v++) {
                    result.assignment[v] = (state[v] == 1);
                }
                break;
            }
        }

        if (!result.satisfiable && !result.assignment.empty()) {
            // I-extract pa rin ang best assignment
            result.assignment.resize(num_vars);
            for (int v = 0; v < num_vars; v++) {
                result.assignment[v] = false;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenPipeline
