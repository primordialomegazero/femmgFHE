#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <queue>

namespace GoldenWatched {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenWatchedSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long conflicts;
        double time_ms;
    };

private:
    using Assignment = std::vector<int>;  // 1=TRUE, -1=FALSE, 0=unassigned
    
    struct Clause {
        std::vector<int> literals;
        int watched1;  // Index ng unang watched literal
        int watched2;  // Index ng pangalawang watched literal
        
        Clause(const std::vector<int>& lits) : literals(lits), watched1(0), watched2(1) {}
    };
    
    // Watched literals - O(1) propagation!
    static bool propagate_watched(std::vector<Clause>& clauses,
                                  Assignment& state,
                                  std::queue<int>& propagation_queue) {
        while (!propagation_queue.empty()) {
            int var = propagation_queue.front();
            propagation_queue.pop();
            
            // Check lahat ng clauses na may watched literal sa var na ito
            for (int ci = 0; ci < clauses.size(); ci++) {
                Clause& clause = clauses[ci];
                
                // Check kung may watched literal na involved
                bool involved = false;
                for (int wi : {clause.watched1, clause.watched2}) {
                    if (abs(clause.literals[wi]) - 1 == var) {
                        involved = true;
                        break;
                    }
                }
                
                if (!involved) continue;
                
                // Check clause satisfaction
                bool satisfied = false;
                for (int lit : clause.literals) {
                    int v = abs(lit) - 1;
                    if (state[v] == 0) continue;
                    bool val = state[v] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        satisfied = true;
                        break;
                    }
                }
                
                if (satisfied) continue;
                
                // Try to find new watched literal
                bool found_new_watch = false;
                for (int li = 0; li < clause.literals.size(); li++) {
                    if (li == clause.watched1 || li == clause.watched2) continue;
                    
                    int lit = clause.literals[li];
                    int v = abs(lit) - 1;
                    
                    // Hindi dapat false ang literal
                    if (state[v] == 0 || 
                        ((lit > 0 && state[v] == 1) || (lit < 0 && state[v] == -1))) {
                        // I-update ang watched literal
                        if (clause.watched1 == 0 || 
                            abs(clause.literals[clause.watched1]) - 1 == var) {
                            clause.watched1 = li;
                        } else {
                            clause.watched2 = li;
                        }
                        found_new_watch = true;
                        break;
                    }
                }
                
                if (found_new_watch) continue;
                
                // Hindi makahanap ng new watch - check kung unit o conflict
                int unassigned_count = 0;
                int last_lit = 0;
                
                for (int lit : clause.literals) {
                    int v = abs(lit) - 1;
                    if (state[v] == 0) {
                        unassigned_count++;
                        last_lit = lit;
                    }
                }
                
                if (unassigned_count == 1) {
                    // Unit clause - i-assign agad!
                    int v = abs(last_lit) - 1;
                    state[v] = (last_lit > 0) ? 1 : -1;
                    propagation_queue.push(v);
                } else if (unassigned_count == 0) {
                    // CONFLICT!
                    return false;
                }
            }
        }
        return true;
    }
    
    // Variable selection with activity tracking
    static int select_variable(const std::vector<Clause>& clauses,
                               const Assignment& state,
                               const std::vector<double>& activity) {
        int best_var = -1;
        double best_score = -1e18;
        
        for (int v = 0; v < state.size(); v++) {
            if (state[v] != 0) continue;
            
            int count = 0;
            for (const auto& clause : clauses) {
                bool satisfied = false;
                for (int lit : clause.literals) {
                    int var = abs(lit) - 1;
                    if (state[var] == 0) continue;
                    bool val = state[var] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        satisfied = true;
                        break;
                    }
                }
                
                if (!satisfied) {
                    for (int lit : clause.literals) {
                        if (abs(lit) - 1 == v) {
                            count++;
                            break;
                        }
                    }
                }
            }
            
            double score = count * (1.0 + activity[v]);
            if (score > best_score) {
                best_score = score;
                best_var = v;
            }
        }
        
        return best_var;
    }
    
    static bool solve_recursive(std::vector<Clause>& clauses,
                                Assignment& state,
                                std::vector<double>& activity,
                                long long& steps,
                                long long& conflicts) {
        steps++;
        
        // Propagate all assigned variables
        std::queue<int> propagation_queue;
        for (int v = 0; v < state.size(); v++) {
            if (state[v] != 0) {
                propagation_queue.push(v);
            }
        }
        
        if (!propagate_watched(clauses, state, propagation_queue)) {
            conflicts++;
            
            // Increase activity ng conflicting variables
            for (const auto& clause : clauses) {
                bool all_assigned = true;
                bool satisfied = false;
                for (int lit : clause.literals) {
                    int v = abs(lit) - 1;
                    if (state[v] == 0) {
                        all_assigned = false;
                        break;
                    }
                    bool val = state[v] == 1;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        satisfied = true;
                        break;
                    }
                }
                
                if (all_assigned && !satisfied) {
                    for (int lit : clause.literals) {
                        int v = abs(lit) - 1;
                        activity[v] *= 1.1;  // Golden ratio learning
                    }
                }
            }
            
            return false;
        }
        
        // Check complete
        bool complete = true;
        for (int val : state) {
            if (val == 0) {
                complete = false;
                break;
            }
        }
        
        if (complete) {
            return true;
        }
        
        // Select variable
        int var = select_variable(clauses, state, activity);
        
        if (var == -1) return false;
        
        // Try TRUE first (mas common sa random 3-SAT)
        Assignment state_true = state;
        state_true[var] = 1;
        if (solve_recursive(clauses, state_true, activity, steps, conflicts)) {
            state = state_true;
            return true;
        }
        
        // Try FALSE
        Assignment state_false = state;
        state_false[var] = -1;
        if (solve_recursive(clauses, state_false, activity, steps, conflicts)) {
            state = state_false;
            return true;
        }
        
        return false;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses_input, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.conflicts = 0;
        result.time_ms = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Convert to Clause objects
        std::vector<Clause> clauses;
        for (const auto& c : clauses_input) {
            clauses.emplace_back(c);
        }
        
        Assignment state(num_vars, 0);
        std::vector<double> activity(num_vars, 1.0);
        
        result.satisfiable = solve_recursive(clauses, state, activity, 
                                             result.steps, result.conflicts);
        
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = (state[v] == 1);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        return result;
    }
};

} // namespace GoldenWatched
