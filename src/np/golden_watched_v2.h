#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <queue>
#include <unordered_map>

namespace GoldenWatchedV2 {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenWatchedSolverV2 {
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
        
        Clause(const std::vector<int>& lits) : literals(lits) {}
    };
    
    // Watch lists: para sa bawat variable, listahan ng clauses na nagwa-watch
    struct Watcher {
        int clause_idx;
        int literal_idx;
        
        Watcher(int ci, int li) : clause_idx(ci), literal_idx(li) {}
    };
    
    // Check kung satisfied ang clause
    static bool is_clause_satisfied(const Clause& clause, const Assignment& state) {
        for (int lit : clause.literals) {
            int var = abs(lit) - 1;
            if (state[var] == 0) continue;
            bool val = state[var] == 1;
            if ((lit > 0 && val) || (lit < 0 && !val)) {
                return true;
            }
        }
        return false;
    }
    
    // Check kung false ang literal sa kasalukuyang assignment
    static bool is_literal_false(int lit, const Assignment& state) {
        int var = abs(lit) - 1;
        if (state[var] == 0) return false;  // Unassigned
        bool val = state[var] == 1;
        return (lit > 0 && !val) || (lit < 0 && val);
    }
    
    // Check kung true ang literal
    static bool is_literal_true(int lit, const Assignment& state) {
        int var = abs(lit) - 1;
        if (state[var] == 0) return false;
        bool val = state[var] == 1;
        return (lit > 0 && val) || (lit < 0 && !val);
    }
    
    // Propagate gamit ang watch lists - O(1) per clause!
    static bool propagate_watch_lists(std::vector<Clause>& clauses,
                                      std::vector<std::vector<Watcher>>& watch_lists,
                                      Assignment& state,
                                      std::queue<int>& propagation_queue) {
        while (!propagation_queue.empty()) {
            int var = propagation_queue.front();
            propagation_queue.pop();
            
            // Kopyahin ang watch list para sa var na ito
            std::vector<Watcher> watchers = watch_lists[var];
            watch_lists[var].clear();
            
            for (const Watcher& w : watchers) {
                Clause& clause = clauses[w.clause_idx];
                
                // Check kung satisfied na
                if (is_clause_satisfied(clause, state)) {
                    // Ibalik sa watch list
                    watch_lists[var].push_back(w);
                    continue;
                }
                
                // Hanapin ang ibang literal na pwedeng i-watch
                bool found_new_watch = false;
                
                for (int li = 0; li < clause.literals.size(); li++) {
                    int lit = clause.literals[li];
                    
                    // Hindi dapat false ang bagong watch
                    if (!is_literal_false(lit, state)) {
                        // I-watch ang bagong literal
                        int new_var = abs(lit) - 1;
                        if (new_var != var) {
                            watch_lists[new_var].push_back(
                                Watcher(w.clause_idx, li)
                            );
                            found_new_watch = true;
                            break;
                        }
                    }
                }
                
                if (found_new_watch) continue;
                
                // Hindi makahanap ng bagong watch - check kung unit o conflict
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
                    
                    // Ibalik sa watch list
                    watch_lists[var].push_back(w);
                } else if (unassigned_count == 0) {
                    // CONFLICT!
                    // Ibalik ang natitirang watchers
                    for (const Watcher& w2 : watchers) {
                        if (w2.clause_idx != w.clause_idx) {
                            watch_lists[var].push_back(w2);
                        }
                    }
                    return false;
                } else {
                    // Ibalik sa watch list
                    watch_lists[var].push_back(w);
                }
            }
        }
        return true;
    }
    
    // Simple variable selection
    static int select_variable(const std::vector<Clause>& clauses,
                               const Assignment& state) {
        int best_var = -1;
        int best_count = -1;
        
        for (int v = 0; v < state.size(); v++) {
            if (state[v] != 0) continue;
            
            int count = 0;
            for (const auto& clause : clauses) {
                if (is_clause_satisfied(clause, state)) continue;
                
                for (int lit : clause.literals) {
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
        
        return best_var;
    }
    
    static bool solve_recursive(std::vector<Clause>& clauses,
                                std::vector<std::vector<Watcher>>& watch_lists,
                                Assignment& state,
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
        
        if (!propagate_watch_lists(clauses, watch_lists, state, propagation_queue)) {
            conflicts++;
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
        int var = select_variable(clauses, state);
        
        if (var == -1) return false;
        
        // Try TRUE
        Assignment state_true = state;
        state_true[var] = 1;
        std::vector<std::vector<Watcher>> watch_lists_true = watch_lists;
        
        if (solve_recursive(clauses, watch_lists_true, state_true, steps, conflicts)) {
            state = state_true;
            watch_lists = watch_lists_true;
            return true;
        }
        
        // Try FALSE
        Assignment state_false = state;
        state_false[var] = -1;
        std::vector<std::vector<Watcher>> watch_lists_false = watch_lists;
        
        if (solve_recursive(clauses, watch_lists_false, state_false, steps, conflicts)) {
            state = state_false;
            watch_lists = watch_lists_false;
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
        
        // Convert sa Clause objects
        std::vector<Clause> clauses;
        for (const auto& c : clauses_input) {
            clauses.emplace_back(c);
        }
        
        // Initialize watch lists
        std::vector<std::vector<Watcher>> watch_lists(num_vars);
        for (int ci = 0; ci < clauses.size(); ci++) {
            // I-watch ang unang dalawang literals
            if (clauses[ci].literals.size() >= 2) {
                int lit1 = clauses[ci].literals[0];
                int lit2 = clauses[ci].literals[1];
                watch_lists[abs(lit1) - 1].push_back(Watcher(ci, 0));
                watch_lists[abs(lit2) - 1].push_back(Watcher(ci, 1));
            } else if (clauses[ci].literals.size() == 1) {
                int lit1 = clauses[ci].literals[0];
                watch_lists[abs(lit1) - 1].push_back(Watcher(ci, 0));
            }
        }
        
        Assignment state(num_vars, 0);
        
        result.satisfiable = solve_recursive(clauses, watch_lists, state, 
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

} // namespace GoldenWatchedV2
