#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>

namespace GoldenEntanglementClassical {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenEntanglementSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long entanglements;
        long long collapses;
        double time_ms;
    };

private:
    // Classical entanglement: variable relationships
    // Kung var A at var B ay nasa parehong clause, entangled sila
    struct Entanglement {
        int var1;
        int var2;
        int relation;  // 1 = same sign, -1 = opposite sign
        // relation = 1: var1 at var2 dapat pareho
        // relation = -1: var1 at var2 dapat magkaiba
    };

    // Build entanglement graph mula sa clauses
    static std::vector<Entanglement> build_entanglements(
        const std::vector<std::vector<int>>& clauses) {
        std::vector<Entanglement> entanglements;
        std::unordered_set<std::string> seen;

        for (const auto& clause : clauses) {
            // Para sa bawat pares ng literals sa clause
            for (int i = 0; i < clause.size(); i++) {
                for (int j = i + 1; j < clause.size(); j++) {
                    int lit1 = clause[i];
                    int lit2 = clause[j];

                    int var1 = abs(lit1) - 1;
                    int var2 = abs(lit2) - 1;

                    // Relation: kung parehong positive o parehong negative
                    int sign1 = (lit1 > 0) ? 1 : -1;
                    int sign2 = (lit2 > 0) ? 1 : -1;

                    // Kung (lit1 OR lit2) ay dapat true, at least isa sa kanila true
                    // Ang entanglement: kung alam natin na isa ay false, yung isa dapat true
                    Entanglement ent;
                    ent.var1 = var1;
                    ent.var2 = var2;
                    ent.relation = sign1 * sign2;  // +1 kung same sign, -1 kung opposite

                    // Deduplicate
                    std::string key = std::to_string(std::min(var1, var2)) + "_" +
                                     std::to_string(std::max(var1, var2));
                    if (seen.find(key) == seen.end()) {
                        seen.insert(key);
                        entanglements.push_back(ent);
                    }
                }
            }
        }

        return entanglements;
    }

    // Propagate gamit ang entanglement: kung alam ang value ng isa, 
    // malalaman ang value ng kabila
    static bool propagate_entanglement(
        const std::vector<std::vector<int>>& clauses,
        std::vector<int>& state,  // 1=TRUE, -1=FALSE, 0=unknown
        const std::vector<Entanglement>& entanglements,
        long long& propagations) {
        
        bool changed = true;
        std::queue<int> to_process;

        // Idagdag lahat ng assigned variables sa queue
        for (int v = 0; v < state.size(); v++) {
            if (state[v] != 0) {
                to_process.push(v);
            }
        }

        while (!to_process.empty()) {
            int var = to_process.front();
            to_process.pop();

            // Hanapin ang lahat ng entanglements na may var na ito
            for (const auto& ent : entanglements) {
                int other = -1;
                if (ent.var1 == var) other = ent.var2;
                else if (ent.var2 == var) other = ent.var1;
                else continue;

                // Kung hindi pa assigned ang other
                if (state[other] == 0) {
                    // Ang relation: para ma-satisfy ang clause,
                    // kung var ay false, other ay dapat true
                    // Kung var ay true, wala tayong masasabi about other
                    if (state[var] == -1) {
                        // Dapat true ang other (para sa positive literal)
                        // Pero depende sa relation
                        state[other] = 1;
                        to_process.push(other);
                        propagations++;
                    }
                    // Kung state[var] == 1, hindi natin alam ang other
                }
            }

            // Check din ang unit clauses
            for (const auto& clause : clauses) {
                int unassigned_count = 0;
                int unassigned_var = -1;
                int unassigned_sign = 0;
                bool satisfied = false;

                for (int lit : clause) {
                    int v = abs(lit) - 1;
                    bool val = state[v] == 1;
                    if (state[v] == 0) {
                        unassigned_count++;
                        unassigned_var = v;
                        unassigned_sign = (lit > 0) ? 1 : -1;
                    } else if ((lit > 0 && val) || (lit < 0 && !val)) {
                        satisfied = true;
                        break;
                    }
                }

                // Kung unit clause at hindi pa satisfied
                if (!satisfied && unassigned_count == 1) {
                    int target_val = (unassigned_sign > 0) ? 1 : -1;
                    if (state[unassigned_var] == 0) {
                        state[unassigned_var] = target_val;
                        to_process.push(unassigned_var);
                        propagations++;
                    } else if (state[unassigned_var] != target_val) {
                        return false;  // Conflict
                    }
                }
            }
        }

        return true;
    }

    // Check kung satisfied na ang formula
    static bool is_satisfied(const std::vector<std::vector<int>>& clauses,
                             const std::vector<int>& state) {
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
            if (!sat) return false;
        }
        return true;
    }

    // Pumili ng variable na i-assign (branching)
    static int choose_branch_var(const std::vector<std::vector<int>>& clauses,
                                 const std::vector<int>& state) {
        // Pumili ng variable na nasa pinakamaraming unsatisfied clauses
        std::unordered_map<int, int> var_count;

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
                for (int lit : clause) {
                    int var = abs(lit) - 1;
                    if (state[var] == 0) {
                        var_count[var]++;
                    }
                }
            }
        }

        if (var_count.empty()) return -1;

        auto max_it = std::max_element(var_count.begin(), var_count.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        return max_it->first;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars,
                        int max_steps = 10000) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.entanglements = 0;
        result.collapses = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Build entanglements
        auto entanglements = build_entanglements(clauses);
        result.entanglements = entanglements.size();

        // State: 0=unknown, 1=TRUE, -1=FALSE
        std::vector<int> state(num_vars, 0);

        // Propagation lamang, walang backtracking
        for (int step = 0; step < max_steps; step++) {
            result.steps = step + 1;

            // I-propagate ang entanglements
            long long props = 0;
            bool valid = propagate_entanglement(clauses, state, entanglements, props);

            if (!valid) {
                // May conflict
                break;
            }

            // Check kung satisfied na
            if (is_satisfied(clauses, state)) {
                result.satisfiable = true;
                break;
            }

            // Pumili ng variable na i-assign
            int var = choose_branch_var(clauses, state);
            if (var == -1) {
                // Wala nang ma-assign pero hindi satisfied
                break;
            }

            // I-assign ang variable
            // Golden ratio heuristic: kung mas maraming positive literals, TRUE
            int pos_count = 0, neg_count = 0;
            for (const auto& clause : clauses) {
                for (int lit : clause) {
                    if (abs(lit) - 1 == var) {
                        if (lit > 0) pos_count++;
                        else neg_count++;
                    }
                }
            }

            state[var] = (neg_count > pos_count) ? -1 : 1;
            result.collapses++;
        }

        // Extract assignment
        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            if (state[v] == 0) {
                result.assignment[v] = false;  // Default sa FALSE kung unknown
            } else {
                result.assignment[v] = (state[v] == 1);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        return result;
    }
};

} // namespace GoldenEntanglementClassical
