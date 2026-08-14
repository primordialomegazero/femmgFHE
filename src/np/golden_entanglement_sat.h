#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <complex>

namespace GoldenEntanglement {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenEntanglementSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        double time_ms;
    };

private:
    // Quantum state: complex amplitude (a + bi)
    struct QuantumVar {
        std::complex<double> amplitude;
        bool measured;
        bool value;
        
        QuantumVar() : amplitude(1.0, 0.0), measured(false), value(false) {}
    };
    
    // Entangled state: φ⊗ψ = -1
    static std::complex<double> entangle(const std::complex<double>& a,
                                         const std::complex<double>& b) {
        // φ·ψ = -1 na entanglement
        return a * b * (PHI * PSI);
    }
    
    // Bell state: (|00⟩ + |11⟩)/√2
    static std::pair<std::complex<double>, std::complex<double>> bell_state() {
        double norm = 1.0 / std::sqrt(2.0);
        return {
            std::complex<double>(norm, 0.0),
            std::complex<double>(norm, 0.0)
        };
    }
    
    // Measure entangled pair
    static void measure_entangled(QuantumVar& q1, QuantumVar& q2) {
        // Kapag nag-measure ng isa, automatic ang isa
        double random = (double)rand() / RAND_MAX;
        bool result = random > 0.5;
        
        q1.measured = true;
        q1.value = result;
        
        // Entangled: ang isa ay kabaligtaran (φ·ψ = -1)
        q2.measured = true;
        q2.value = !result;
    }
    
    static bool clause_satisfied(const std::vector<int>& clause,
                                 const std::vector<QuantumVar>& state) {
        for (int lit : clause) {
            int var = abs(lit) - 1;
            if (!state[var].measured) continue;
            
            bool val = state[var].value;
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                return true;
            }
        }
        return false;
    }
    
    // Entangled propagation: kapag nag-assign, i-update ang entangled vars
    static void propagate_entangled(const std::vector<std::vector<int>>& clauses,
                                    std::vector<QuantumVar>& state,
                                    int assigned_var) {
        // Hanapin ang clauses na may assigned_var
        for (const auto& clause : clauses) {
            bool has_var = false;
            for (int lit : clause) {
                if (abs(lit) - 1 == assigned_var) {
                    has_var = true;
                    break;
                }
            }
            
            if (!has_var) continue;
            
            // Check kung may unassigned vars sa clause
            int unassigned_count = 0;
            int last_unassigned = -1;
            
            for (int lit : clause) {
                int var = abs(lit) - 1;
                if (!state[var].measured) {
                    unassigned_count++;
                    last_unassigned = var;
                }
            }
            
            // Unit clause: i-assign ang last unassigned
            if (unassigned_count == 1 && last_unassigned != -1) {
                // Hanapin ang tamang value
                bool clause_sat = false;
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (state[var].measured) {
                        bool val = state[var].value;
                        if ((lit > 0 && val) || (lit < 0 && !val)) {
                            clause_sat = true;
                            break;
                        }
                    }
                }
                
                if (!clause_sat) {
                    // Kailangang i-assign ang last_unassigned para ma-satisfy
                    for (int lit : clause) {
                        if (abs(lit) - 1 == last_unassigned) {
                            state[last_unassigned].measured = true;
                            state[last_unassigned].value = (lit > 0);
                            break;
                        }
                    }
                }
            }
        }
    }
    
    static bool solve_recursive(const std::vector<std::vector<int>>& clauses,
                                std::vector<QuantumVar>& state,
                                long long& steps) {
        steps++;
        
        // Check kung complete
        bool complete = true;
        for (const auto& q : state) {
            if (!q.measured) {
                complete = false;
                break;
            }
        }
        
        if (complete) {
            for (const auto& clause : clauses) {
                if (!clause_satisfied(clause, state)) {
                    return false;
                }
            }
            return true;
        }
        
        // Hanapin ang unassigned variable na may pinakamaraming constraints
        int best_var = -1;
        int best_count = -1;
        
        for (int v = 0; v < state.size(); v++) {
            if (state[v].measured) continue;
            
            int count = 0;
            for (const auto& clause : clauses) {
                if (clause_satisfied(clause, state)) continue;
                for (int lit : clause) {
                    if (abs(lit) - 1 == v) {
                        count++;
                        break;
                    }
                }
            }
            
            if (count > best_count) {
                best_count = count;
                best_var = v;
            }
        }
        
        if (best_var == -1) return false;
        
        // Try TRUE with entanglement
        std::vector<QuantumVar> state_true = state;
        state_true[best_var].measured = true;
        state_true[best_var].value = true;
        propagate_entangled(clauses, state_true, best_var);
        
        if (solve_recursive(clauses, state_true, steps)) {
            state = state_true;
            return true;
        }
        
        // Try FALSE with entanglement
        std::vector<QuantumVar> state_false = state;
        state_false[best_var].measured = true;
        state_false[best_var].value = false;
        propagate_entangled(clauses, state_false, best_var);
        
        if (solve_recursive(clauses, state_false, steps)) {
            state = state_false;
            return true;
        }
        
        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<QuantumVar> state(num_vars);
        
        result.satisfiable = solve_recursive(clauses, state, result.steps);
        
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = state[v].value;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenEntanglement
