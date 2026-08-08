#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct FractalMirrorSearch {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    std::vector<double> observer;
    std::vector<double> observed;
    int decisions;
    int nodes_explored;

    FractalMirrorSearch(int n, const std::vector<std::vector<int>>& cls)
        : n_vars(n), clauses(cls), decisions(0), nodes_explored(0) {
        assignment.resize(n_vars + 1, 0);
        observer.resize(n_vars + 1, PHI);
        observed.resize(n_vars + 1, PSI);
    }

    bool is_sat() {
        decisions++;
        return backtrack(1);
    }

    int select_branch(int var) {
        double sum = observer[var] + observed[var];
        return (sum >= 0) ? 1 : -1;
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

    bool backtrack(int var) {
        nodes_explored++;
        if (var > n_vars) {
            return check_assignment();
        }

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

    bool check_assignment() {
        for (const auto& clause : clauses) {
            bool sat = false;
            for (int lit : clause) {
                int v = abs(lit);
                int val = assignment[v];
                if (val == 0) return false;
                if ((lit > 0 && val == 1) || (lit < 0 && val == -1)) {
                    sat = true;
                    break;
                }
            }
            if (!sat) return false;
        }
        return true;
    }
};

double sublinear_formula(double n) {
    return (1.0 / PHI) * pow(n, 1.0 / PHI);
}

int main() {
    std::vector<std::vector<int>> php3 = {
        {1,2}, {1,3}, {2,3}, {-1,-2}, {-1,-3}, {-2,-3}
    };

    FractalMirrorSearch solver(3, php3);
    bool result = solver.is_sat();

    double sn = sublinear_formula(3);

    std::cout << "\n";
    std::cout << "  PHP_3 (3 vars, 6 clauses)\n";
    std::cout << "  Result: " << (result ? "SAT" : "UNSAT") << "\n";
    std::cout << "  Decisions: " << solver.decisions << "\n";
    std::cout << "  Nodes Explored: " << solver.nodes_explored << "\n";
    std::cout << "  S(3) = " << std::fixed << std::setprecision(4) << sn << "\n";
    std::cout << "  Sub-linear: " << (solver.nodes_explored <= sn ? "YES" : "NO") << "\n\n";

    return 0;
}
