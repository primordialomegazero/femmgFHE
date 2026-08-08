#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct FractalMirrorDebug {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    std::vector<double> observer;
    std::vector<double> observed;
    int decisions;
    int nodes_explored;
    int propagations;
    int conflicts;
    bool verbose;

    FractalMirrorDebug(int n, const std::vector<std::vector<int>>& cls, bool v)
        : n_vars(n), clauses(cls), decisions(0), nodes_explored(0), 
          propagations(0), conflicts(0), verbose(v) {
        assignment.resize(n_vars + 1, 0);
        observer.resize(n_vars + 1, PHI);
        observed.resize(n_vars + 1, PSI);
    }

    int lit_val(int lit) {
        int v = abs(lit);
        if (assignment[v] == 0) return 0;
        return (assignment[v] == 1) ? (lit > 0 ? 1 : -1) : (lit > 0 ? -1 : 1);
    }

    bool clause_sat(const std::vector<int>& cl) {
        for (int lit : cl) if (lit_val(lit) == 1) return true;
        return false;
    }

    void mirror_update(int var, int value) {
        double obs_factor = (value == 1) ? PHI : PSI;
        double obd_factor = (value == 1) ? PSI : PHI;
        observer[var] *= obs_factor;
        observed[var] *= obd_factor;
        double sum = observer[var] + observed[var];
        if (fabs(sum) > 1e-12) {
            observer[var] /= sum;
            observed[var] /= sum;
        }

        double total_obs = 0, total_obd = 0, total_weight = 0;
        for (int u = 1; u <= n_vars; u++) {
            if (assignment[u] != 0 && u != var) {
                double dist = fabs(var - u);
                double weight = 1.0 / (1.0 + dist * dist / PHI);
                total_obs += observer[u] * weight;
                total_obd += observed[u] * weight;
                total_weight += weight;
            }
        }
        if (total_weight > 0) {
            double self_w = PHI;
            observer[var] = (observer[var] * self_w + total_obs) / (self_w + total_weight);
            observed[var] = (observed[var] * self_w + total_obd) / (self_w + total_weight);
            double ns = observer[var] + observed[var];
            if (fabs(ns) > 1e-12) {
                observer[var] /= ns;
                observed[var] /= ns;
            }
        }
    }

    bool propagate() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (int ci = 0; ci < (int)clauses.size(); ci++) {
                auto& cl = clauses[ci];
                if (clause_sat(cl)) continue;
                int unassigned = 0, last_lit = 0, false_count = 0;
                for (int lit : cl) {
                    int val = lit_val(lit);
                    if (val == 0) { unassigned++; last_lit = lit; }
                    else if (val == -1) false_count++;
                }
                if (unassigned == 0 && false_count == (int)cl.size()) {
                    conflicts++;
                    if (verbose) {
                        std::cout << "    CONFLICT clause " << ci << ": ";
                        for (int lit : cl) std::cout << lit << "(" << lit_val(lit) << ") ";
                        std::cout << "\n";
                    }
                    return false;
                }
                if (unassigned == 1 && false_count == (int)cl.size() - 1) {
                    int v = abs(last_lit), val = (last_lit > 0) ? 1 : -1;
                    if (assignment[v] != 0 && assignment[v] != val) {
                        conflicts++;
                        if (verbose) {
                            std::cout << "    CONFLICT unit clause " << ci << ": var " << v 
                                      << " assigned " << assignment[v] << " need " << val << "\n";
                        }
                        return false;
                    }
                    if (assignment[v] == 0) {
                        assignment[v] = val;
                        propagations++;
                        mirror_update(v, val);
                        changed = true;
                        if (verbose) {
                            std::cout << "    PROPAGATE var " << v << " = " << val 
                                      << " (clause " << ci << ")\n";
                        }
                    }
                }
            }
        }
        return true;
    }

    int select_branch(int var) {
        double sum = observer[var] + observed[var];
        return (sum >= 0) ? 1 : -1;
    }

    void print_state(int depth, const std::string& action) {
        std::cout << "  [d=" << depth << " " << action << "] ";
        for (int v = 1; v <= n_vars; v++) {
            std::cout << "x" << v << "=";
            if (assignment[v] == 0) std::cout << "?";
            else if (assignment[v] == 1) std::cout << "T";
            else std::cout << "F";
            std::cout << "(" << std::fixed << std::setprecision(2) << observer[v] 
                      << "/" << observed[v] << ") ";
        }
        std::cout << "\n";
    }

    bool backtrack(int var, int depth) {
        nodes_explored++;
        if (verbose) print_state(depth, "ENTER");

        if (!propagate()) {
            if (verbose) std::cout << "    BACKTRACK (conflict)\n";
            return false;
        }

        if (var > n_vars) {
            bool sat = true;
            for (auto& cl : clauses) if (!clause_sat(cl)) { sat = false; break; }
            if (verbose) std::cout << "    LEAF: " << (sat ? "SAT" : "UNSAT") << "\n";
            return sat;
        }

        if (assignment[var] != 0) {
            return backtrack(var + 1, depth + 1);
        }

        decisions++;
        int first_val = select_branch(var);

        assignment[var] = first_val;
        mirror_update(var, first_val);
        if (backtrack(var + 1, depth + 1)) return true;

        assignment[var] = -first_val;
        mirror_update(var, -first_val);
        if (backtrack(var + 1, depth + 1)) return true;

        assignment[var] = 0;
        if (verbose) std::cout << "    BACKTRACK (both failed)\n";
        return false;
    }

    bool is_sat() {
        return backtrack(1, 0);
    }
};

double sublinear_formula(double n) {
    return (1.0 / PHI) * pow(n, 1.0 / PHI);
}

int main() {
    std::vector<std::vector<int>> php3 = {
        {1,2}, {1,3}, {2,3}, {-1,-2}, {-1,-3}, {-2,-3}
    };

    std::cout << "\nPHP_3 VERBOSE TRACE:\n\n";

    FractalMirrorDebug solver(3, php3, true);
    bool result = solver.is_sat();

    double sn = sublinear_formula(3);
    std::cout << "\n";
    std::cout << "  Result: " << (result ? "SAT" : "UNSAT") << "\n";
    std::cout << "  Decisions: " << solver.decisions << "\n";
    std::cout << "  Nodes Explored: " << solver.nodes_explored << "\n";
    std::cout << "  Propagations: " << solver.propagations << "\n";
    std::cout << "  Conflicts: " << solver.conflicts << "\n";
    std::cout << "  S(3) = " << std::fixed << std::setprecision(4) << sn << "\n\n";

    return 0;
}
