#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct GoldenFibonacciMirror {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    std::vector<double> observer;
    std::vector<double> observed;
    std::vector<int> fib_index;
    int decisions;
    int nodes_explored;
    int propagations;
    int conflicts;

    GoldenFibonacciMirror(int n, const std::vector<std::vector<int>>& cls)
        : n_vars(n), clauses(cls), decisions(0), nodes_explored(0),
          propagations(0), conflicts(0) {
        assignment.resize(n_vars + 1, 0);
        observer.resize(n_vars + 1, PHI);
        observed.resize(n_vars + 1, PSI);
        fib_index.resize(n_vars + 1);
        for (int v = 1; v <= n_vars; v++) {
            fib_index[v] = v;
        }
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

    double fib_weight(int a, int b) {
        int diff = abs(fib_index[a] - fib_index[b]);
        if (diff == 0) return PHI;
        double f1 = pow(PHI, diff);
        double f2 = pow(fabs(PSI), diff);
        if (diff % 2 == 1) f2 = -f2;
        double fib = (f1 - f2) / sqrt(5.0);
        return 1.0 / (1.0 + fabs(fib) / PHI);
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
                double weight = fib_weight(var, u);
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

        for (int u = 1; u <= n_vars; u++) {
            if (assignment[u] != 0 && u != var) {
                fib_index[u] = fib_index[u] * PHI + fib_index[var] * fabs(PSI);
                fib_index[u] = abs(fib_index[u]) % n_vars;
                if (fib_index[u] == 0) fib_index[u] = n_vars;
            }
        }
    }

    int select_branch(int var) {
        double sum = observer[var] + observed[var];
        return (sum >= 0) ? 1 : -1;
    }

    bool propagate() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (auto& cl : clauses) {
                if (clause_sat(cl)) continue;
                int unassigned = 0, last_lit = 0, false_count = 0;
                for (int lit : cl) {
                    int val = lit_val(lit);
                    if (val == 0) { unassigned++; last_lit = lit; }
                    else if (val == -1) false_count++;
                }
                if (unassigned == 0 && false_count == (int)cl.size()) {
                    conflicts++;
                    return false;
                }
                if (unassigned == 1 && false_count == (int)cl.size() - 1) {
                    int v = abs(last_lit), val = (last_lit > 0) ? 1 : -1;
                    if (assignment[v] != 0 && assignment[v] != val) {
                        conflicts++;
                        return false;
                    }
                    if (assignment[v] == 0) {
                        assignment[v] = val;
                        propagations++;
                        mirror_update(v, val);
                        changed = true;
                    }
                }
            }
        }
        return true;
    }

    bool backtrack(int var) {
        nodes_explored++;

        if (!propagate()) return false;

        if (var > n_vars) {
            for (auto& cl : clauses) if (!clause_sat(cl)) return false;
            return true;
        }

        if (assignment[var] != 0) return backtrack(var + 1);

        decisions++;
        int first_val = select_branch(var);

        assignment[var] = first_val;
        mirror_update(var, first_val);
        if (backtrack(var + 1)) return true;

        assignment[var] = -first_val;
        mirror_update(var, -first_val);
        if (backtrack(var + 1)) return true;

        assignment[var] = 0;
        return false;
    }

    bool is_sat() {
        return backtrack(1);
    }
};

double sublinear_formula(double n) {
    return (1.0 / PHI) * pow(n, 1.0 / PHI);
}

std::vector<std::vector<int>> gen_php(int n) {
    std::vector<std::vector<int>> cls;
    for (int p = 0; p < n+1; p++) {
        std::vector<int> cl;
        for (int h = 0; h < n; h++) cl.push_back(p * n + h + 1);
        cls.push_back(cl);
    }
    for (int h = 0; h < n; h++)
        for (int p1 = 0; p1 < n+1; p1++)
            for (int p2 = p1+1; p2 < n+1; p2++)
                cls.push_back({-(p1 * n + h + 1), -(p2 * n + h + 1)});
    return cls;
}

int main() {
    std::cout << "\n";
    std::cout << "  GOLDEN FIBONACCI FRACTAL MIRROR\n";
    std::cout << "  Pigeonhole Problem\n\n";
    std::cout << "  " << std::left << std::setw(10) << "PHP_n"
              << std::setw(10) << "Vars"
              << std::setw(12) << "Result"
              << std::setw(12) << "Decisions"
              << std::setw(12) << "Nodes"
              << std::setw(12) << "S(n)"
              << std::setw(12) << "Sub-lin?"
              << "\n";
    std::cout << "  " << std::string(80, '-') << "\n";

    for (int n = 2; n <= 15; n++) {
        auto php = gen_php(n);
        int vars = (n + 1) * n;
        GoldenFibonacciMirror solver(vars, php);
        bool result = solver.is_sat();
        double sn = sublinear_formula(vars);
        bool sublinear = (solver.nodes_explored <= sn * 10);

        std::cout << "  " << std::left << std::setw(10) << ("PHP_"+std::to_string(n))
                  << std::setw(10) << vars
                  << std::setw(12) << (result ? "SAT" : "UNSAT")
                  << std::setw(12) << solver.decisions
                  << std::setw(12) << solver.nodes_explored
                  << std::setw(12) << std::fixed << std::setprecision(2) << sn
                  << std::setw(12) << (sublinear ? "YES" : "NO")
                  << "\n";
    }
    std::cout << "\n";
    return 0;
}
