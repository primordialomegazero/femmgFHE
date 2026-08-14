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

namespace GoldenSoftEntangle {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenSoftSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long propagations;
        long long collapses;
        double time_ms;
    };

private:
    // Soft entanglement: variable relationships na may weights
    struct SoftEnt {
        int var1;
        int var2;
        double weight;  // gaano kalakas ang correlation
    };

    // Build soft entanglements mula sa clauses
    static std::vector<SoftEnt> build_soft_entanglements(
        const std::vector<std::vector<int>>& clauses,
        int num_vars) {
        
        std::vector<SoftEnt> ents;
        std::vector<std::vector<int>> var_occurrences(num_vars);
        
        // Bilangin ang co-occurrence ng variables sa clauses
        for (const auto& clause : clauses) {
            for (int lit1 : clause) {
                for (int lit2 : clause) {
                    if (lit1 == lit2) continue;
                    int var1 = abs(lit1) - 1;
                    int var2 = abs(lit2) - 1;
                    var_occurrences[var1].push_back(var2);
                }
            }
        }
        
        // Compute weights batay sa frequency
        std::unordered_map<std::string, int> pair_count;
        for (int v1 = 0; v1 < num_vars; v1++) {
            for (int v2 : var_occurrences[v1]) {
                if (v1 < v2) {
                    std::string key = std::to_string(v1) + "_" + std::to_string(v2);
                    pair_count[key]++;
                }
            }
        }
        
        for (const auto& [key, count] : pair_count) {
            int underscore = key.find("_");
            int v1 = std::stoi(key.substr(0, underscore));
            int v2 = std::stoi(key.substr(underscore + 1));
            
            SoftEnt ent;
            ent.var1 = v1;
            ent.var2 = v2;
            ent.weight = (double)count / 10.0;  // Normalize
            if (ent.weight > 1.0) ent.weight = 1.0;
            
            ents.push_back(ent);
        }
        
        return ents;
    }

    // Soft propagation: i-update ang probabilities, hindi hard values
    static void soft_propagate(
        const std::vector<std::vector<int>>& clauses,
        std::vector<double>& probs,  // Probability na TRUE (0 to 1)
        const std::vector<SoftEnt>& ents,
        int num_vars,
        long long& propagation_count) {
        
        // Initialize probs sa golden ratio
        for (int v = 0; v < num_vars; v++) {
            probs[v] = 0.618;  // φ - 1
        }
        
        // Ulitin ang propagation hanggang mag-stabilize
        for (int iter = 0; iter < 20; iter++) {
            std::vector<double> new_probs = probs;
            
            // Update batay sa clause satisfaction
            for (const auto& clause : clauses) {
                // Compute probability na satisfied ang clause
                double prob_unsat = 1.0;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    double prob_true = probs[var];
                    double prob_lit_true = (lit > 0) ? prob_true : (1 - prob_true);
                    prob_unsat *= (1 - prob_lit_true);
                }
                
                // Kung mataas ang probability na unsat, i-adjust ang variables
                if (prob_unsat > 0.3) {
                    for (int lit : clause) {
                        int var = abs(lit) - 1;
                        if (lit > 0) {
                            // Taasan ang probability na TRUE
                            new_probs[var] += 0.1 * prob_unsat;
                        } else {
                            // Taasan ang probability na FALSE
                            new_probs[var] -= 0.1 * prob_unsat;
                        }
                    }
                }
            }
            
            // Update batay sa entanglements
            for (const auto& ent : ents) {
                // Kung correlated ang dalawang variables
                double avg = (new_probs[ent.var1] + new_probs[ent.var2]) / 2.0;
                new_probs[ent.var1] += ent.weight * (avg - new_probs[ent.var1]);
                new_probs[ent.var2] += ent.weight * (avg - new_probs[ent.var2]);
            }
            
            // Clamp probabilities
            for (int v = 0; v < num_vars; v++) {
                if (new_probs[v] > 0.95) new_probs[v] = 0.95;
                if (new_probs[v] < 0.05) new_probs[v] = 0.05;
            }
            
            // Check convergence
            double change = 0.0;
            for (int v = 0; v < num_vars; v++) {
                change += std::abs(new_probs[v] - probs[v]);
            }
            
            probs = new_probs;
            propagation_count++;
            
            if (change < 0.01) break;
        }
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars,
                        int max_attempts = 50) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.propagations = 0;
        result.collapses = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Build soft entanglements
        auto ents = build_soft_entanglements(clauses, num_vars);

        // Multiple attempts
        std::random_device rd;
        std::mt19937 gen(42);
        std::uniform_real_distribution<> dis(0.0, 1.0);

        for (int attempt = 0; attempt < max_attempts && !result.satisfiable; attempt++) {
            result.steps = attempt + 1;

            // Soft propagation
            std::vector<double> probs(num_vars);
            soft_propagate(clauses, probs, ents, num_vars, result.propagations);

            // I-convert ang probabilities sa assignment
            std::vector<int> state(num_vars);
            for (int v = 0; v < num_vars; v++) {
                state[v] = (probs[v] > 0.5) ? 1 : -1;
            }

            // Check kung satisfied
            bool sat = true;
            for (const auto& clause : clauses) {
                bool clause_sat = false;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    bool val = state[var] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        clause_sat = true;
                        break;
                    }
                }
                if (!clause_sat) {
                    sat = false;
                    break;
                }
            }

            if (sat) {
                result.satisfiable = true;
                result.assignment.resize(num_vars);
                for (int v = 0; v < num_vars; v++) {
                    result.assignment[v] = (state[v] == 1);
                }
                result.collapses = attempt + 1;
                break;
            }

            // Kung malapit na, subukan ang local refinement
            int unsat_count = 0;
            for (const auto& clause : clauses) {
                bool clause_sat = false;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    bool val = state[var] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        clause_sat = true;
                        break;
                    }
                }
                if (!clause_sat) unsat_count++;
            }

            // Kung malapit na (less than 5% unsat), gawin ang local search
            if (unsat_count < clauses.size() * 0.05) {
                // Simple local search
                for (int flip = 0; flip < 100; flip++) {
                    int best_var = -1;
                    int best_unsat = unsat_count + 1;

                    for (int v = 0; v < std::min(num_vars, 20); v++) {
                        state[v] = 1 - state[v];
                        int new_unsat = 0;
                        for (const auto& clause : clauses) {
                            bool clause_sat = false;
                            for (int lit : clause) {
                                int var2 = abs(lit) - 1;
                                bool val2 = state[var2] == 1;
                                if ((lit > 0 && val2) || (lit < 0 && !val2)) {
                                    clause_sat = true;
                                    break;
                                }
                            }
                            if (!clause_sat) new_unsat++;
                        }
                        state[v] = 1 - state[v];

                        if (new_unsat < best_unsat) {
                            best_unsat = new_unsat;
                            best_var = v;
                        }
                    }

                    if (best_var == -1) break;

                    state[best_var] = 1 - state[best_var];
                    unsat_count = best_unsat;

                    if (unsat_count == 0) {
                        result.satisfiable = true;
                        result.assignment.resize(num_vars);
                        for (int v = 0; v < num_vars; v++) {
                            result.assignment[v] = (state[v] == 1);
                        }
                        result.collapses = attempt + 1;
                        break;
                    }
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenSoftEntangle
