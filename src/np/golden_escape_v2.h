#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <random>
#include <algorithm>
#include <unordered_set>

namespace GoldenEscapeV2 {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenEscapeSolverV2 {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long flips;
        double time_ms;
    };

private:
    // Count unsatisfied clauses
    static int count_unsat(const std::vector<std::vector<int>>& clauses,
                           const std::vector<int>& state) {
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
    }
    
    // Get best variable to flip
    static int best_flip(const std::vector<std::vector<int>>& clauses,
                         const std::vector<int>& state,
                         const std::unordered_set<int>& tabu) {
        int best_var = -1;
        int best_improvement = -999999;
        
        for (int v = 0; v < state.size(); v++) {
            if (tabu.count(v)) continue;
            
            std::vector<int> flipped = state;
            flipped[v] = -flipped[v];
            
            int current_unsat = count_unsat(clauses, state);
            int new_unsat = count_unsat(clauses, flipped);
            int improvement = current_unsat - new_unsat;
            
            if (improvement > best_improvement) {
                best_improvement = improvement;
                best_var = v;
            }
        }
        
        return best_var;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.flips = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Initialize sa random assignment
        std::random_device rd;
        std::mt19937 gen(rd());
        std::vector<int> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            state[v] = (gen() % 2 == 0) ? 1 : -1;
        }
        
        std::unordered_set<int> tabu;
        
        // Local search with tabu
        for (int iter = 0; iter < 1000; iter++) {
            result.steps++;
            
            int unsat = count_unsat(clauses, state);
            
            if (unsat == 0) {
                result.satisfiable = true;
                break;
            }
            
            // Find best flip
            int var = best_flip(clauses, state, tabu);
            
            if (var == -1) {
                // Walang ma-flip - clear tabu at random restart
                tabu.clear();
                for (int v = 0; v < num_vars; v++) {
                    state[v] = (gen() % 2 == 0) ? 1 : -1;
                }
                continue;
            }
            
            // Flip variable
            state[var] = -state[var];
            tabu.insert(var);
            result.flips++;
            
            // Limit tabu size
            if (tabu.size() > num_vars / 2) {
                tabu.erase(tabu.begin());
            }
        }
        
        // Extract assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] == 1);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenEscapeV2
