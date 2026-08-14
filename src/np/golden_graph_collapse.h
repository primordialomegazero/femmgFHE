#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <chrono>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace GoldenGraphCollapse {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenGraphSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        long long steps;
        long long collapses;
        double time_ms;
    };

private:
    // Build clause-variable incidence graph
    struct Graph {
        // Para sa bawat variable, aling clauses ang naglalaman nito
        std::vector<std::vector<int>> var_to_clauses;
        // Para sa bawat clause, aling variables ang nasa loob
        std::vector<std::vector<int>> clause_to_vars;
        // Para sa bawat variable, ilang beses lumalabas (positive o negative)
        std::vector<int> var_positive_count;
        std::vector<int> var_negative_count;

        Graph(int num_vars, int num_clauses) {
            var_to_clauses.resize(num_vars);
            clause_to_vars.resize(num_clauses);
            var_positive_count.resize(num_vars, 0);
            var_negative_count.resize(num_vars, 0);
        }
    };

    static Graph build_graph(const std::vector<std::vector<int>>& clauses,
                             int num_vars) {
        Graph graph(num_vars, clauses.size());

        for (int ci = 0; ci < clauses.size(); ci++) {
            for (int lit : clauses[ci]) {
                int var = abs(lit) - 1;
                graph.var_to_clauses[var].push_back(ci);
                graph.clause_to_vars[ci].push_back(var);

                if (lit > 0) {
                    graph.var_positive_count[var]++;
                } else {
                    graph.var_negative_count[var]++;
                }
            }
        }

        return graph;
    }

    // Hanapin ang "collapse set" — variables na sabay-sabay na dapat i-flip
    static std::vector<int> find_collapse_set(const std::vector<std::vector<int>>& clauses,
                                              const std::vector<int>& state,
                                              const Graph& graph,
                                              int num_vars) {
        std::vector<int> unsatisfied_clauses;
        for (int ci = 0; ci < clauses.size(); ci++) {
            bool sat = false;
            for (int lit : clauses[ci]) {
                int var = abs(lit) - 1;
                bool val = state[var] == 1;
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    sat = true;
                    break;
                }
            }
            if (!sat) unsatisfied_clauses.push_back(ci);
        }

        if (unsatisfied_clauses.empty()) return {};

        // Hanapin ang "choke points" — variables na nasa maraming unsatisfied clauses
        std::unordered_map<int, int> var_impact;
        for (int ci : unsatisfied_clauses) {
            for (int var : graph.clause_to_vars[ci]) {
                var_impact[var]++;
            }
        }

        // Piliin ang variables na may pinaka-mataas na impact
        std::vector<std::pair<int, int>> impacts(var_impact.begin(), var_impact.end());
        std::sort(impacts.begin(), impacts.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });

        // Kunin ang top 20% na may pinaka-mataas na impact
        std::vector<int> collapse_set;
        int top_count = std::max(1, (int)(impacts.size() * 0.2));
        for (int i = 0; i < top_count && i < impacts.size(); i++) {
            collapse_set.push_back(impacts[i].first);
        }

        return collapse_set;
    }

    // Check kung ang collapse set ay may golden ratio property
    static bool has_golden_property(const std::vector<int>& collapse_set,
                                    const Graph& graph) {
        if (collapse_set.empty()) return false;

        // Ang golden ratio property: φ² = φ + 1
        // Sa SAT terms: ang collapse set ay dapat may balanseng positive/negative
        double total_positive = 0;
        double total_negative = 0;

        for (int var : collapse_set) {
            total_positive += graph.var_positive_count[var];
            total_negative += graph.var_negative_count[var];
        }

        if (total_positive + total_negative == 0) return false;

        double ratio = total_positive / (total_positive + total_negative);
        // Ang golden ratio ay ~0.618, dapat malapit dito
        return std::abs(ratio - 0.618) < 0.2;
    }

public:
    static Result solve(const std::vector<std::vector<int>>& clauses,
                        int num_vars,
                        int max_steps = 10000) {
        Result result;
        result.satisfiable = false;
        result.steps = 0;
        result.collapses = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Build graph
        Graph graph = build_graph(clauses, num_vars);

        // Initialize state gamit ang golden ratio
        std::vector<int> state(num_vars);
        for (int v = 0; v < num_vars; v++) {
            // Kung mas maraming negative occurrences, gawing FALSE
            if (graph.var_negative_count[v] > graph.var_positive_count[v]) {
                state[v] = 0;
            } else {
                state[v] = 1;
            }
        }

        // Check kung satisfied na agad
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

        if (unsat == 0) {
            result.satisfiable = true;
            result.assignment.resize(num_vars);
            for (int v = 0; v < num_vars; v++) {
                result.assignment[v] = (state[v] == 1);
            }
            auto end = std::chrono::high_resolution_clock::now();
            result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            return result;
        }

        // Iterative collapse
        for (int step = 0; step < max_steps; step++) {
            result.steps = step + 1;

            // Hanapin ang collapse set
            std::vector<int> collapse_set = find_collapse_set(clauses, state, graph, num_vars);

            if (collapse_set.empty()) {
                result.satisfiable = true;
                break;
            }

            // I-collapse ang set
            if (has_golden_property(collapse_set, graph)) {
                // Flip lahat ng variables sa collapse set
                for (int var : collapse_set) {
                    state[var] = 1 - state[var];
                }
                result.collapses++;
            } else {
                // Flip lang ang top variable
                state[collapse_set[0]] = 1 - state[collapse_set[0]];
                result.collapses++;
            }

            // Check kung satisfied na
            unsat = 0;
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

            if (unsat == 0) {
                result.satisfiable = true;
                break;
            }

            // Kung stuck, random restart na may golden influence
            if (step > 100 && step % 50 == 0) {
                for (int v = 0; v < num_vars; v++) {
                    if (graph.var_negative_count[v] > graph.var_positive_count[v]) {
                        state[v] = 0;
                    } else {
                        state[v] = 1;
                    }
                }
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

} // namespace GoldenGraphCollapse
