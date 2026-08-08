#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>
#include <set>

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
        for (int v = 1; v <= n_vars; v++) fib_index[v] = v;
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
        if (fabs(sum) > 1e-12) { observer[var] /= sum; observed[var] /= sum; }

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
            if (fabs(ns) > 1e-12) { observer[var] /= ns; observed[var] /= ns; }
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
                if (unassigned == 0 && false_count == (int)cl.size()) { conflicts++; return false; }
                if (unassigned == 1 && false_count == (int)cl.size() - 1) {
                    int v = abs(last_lit), val = (last_lit > 0) ? 1 : -1;
                    if (assignment[v] != 0 && assignment[v] != val) { conflicts++; return false; }
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
        assignment[var] = first_val; mirror_update(var, first_val);
        if (backtrack(var + 1)) return true;
        assignment[var] = -first_val; mirror_update(var, -first_val);
        if (backtrack(var + 1)) return true;
        assignment[var] = 0;
        return false;
    }

    bool solve() { return backtrack(1); }
};

double sublinear_formula(double n) {
    return (1.0 / PHI) * pow(n, 1.0 / PHI);
}

std::vector<std::vector<int>> gen_php(int n) {
    std::vector<std::vector<int>> cls;
    int holes = n - 1;
    for (int h = 0; h < holes; h++) {
        std::vector<int> cl;
        for (int p = 0; p < n; p++) cl.push_back(p * holes + h + 1);
        cls.push_back(cl);
    }
    for (int h = 0; h < holes; h++)
        for (int p1 = 0; p1 < n; p1++)
            for (int p2 = p1 + 1; p2 < n; p2++)
                cls.push_back({-(p1 * holes + h + 1), -(p2 * holes + h + 1)});
    return cls;
}

std::vector<std::vector<int>> gen_kcol(int v, int c) {
    std::vector<std::vector<int>> cls;
    for (int i = 0; i < v; i++) {
        std::vector<int> cl;
        for (int j = 0; j < c; j++) cl.push_back(i * c + j + 1);
        cls.push_back(cl);
    }
    for (int i = 0; i < v; i++)
        for (int j1 = 0; j1 < c; j1++)
            for (int j2 = j1+1; j2 < c; j2++)
                cls.push_back({-(i * c + j1 + 1), -(i * c + j2 + 1)});
    for (int i1 = 0; i1 < v; i1++)
        for (int i2 = i1+1; i2 < v; i2++)
            for (int j = 0; j < c; j++)
                cls.push_back({-(i1 * c + j + 1), -(i2 * c + j + 1)});
    return cls;
}

std::vector<std::vector<int>> gen_3sat(int n_vars, int n_clauses, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> var_dist(1, n_vars);
    std::uniform_int_distribution<int> sign_dist(0, 1);
    std::vector<std::vector<int>> cls;
    for (int c = 0; c < n_clauses; c++) {
        std::set<int> used;
        std::vector<int> cl;
        while (cl.size() < 3) {
            int v = var_dist(rng);
            if (used.insert(v).second) cl.push_back(sign_dist(rng) ? v : -v);
        }
        cls.push_back(cl);
    }
    return cls;
}

int main() {
    std::cout << "\n";
    std::cout << "  GOLDEN FIBONACCI MIRROR — FULL TEST\n";
    std::cout << "  " << std::string(72, '=') << "\n\n";

    std::cout << "  " << std::left << std::setw(14) << "Test"
              << std::setw(8) << "Vars"
              << std::setw(10) << "Expected"
              << std::setw(10) << "Result"
              << std::setw(10) << "Decisions"
              << std::setw(8) << "Nodes"
              << std::setw(8) << "Prop"
              << std::setw(8) << "Confl"
              << std::setw(10) << "S(n)"
              << "\n";
    std::cout << "  " << std::string(90, '-') << "\n";

    struct Test { std::string name; std::vector<std::vector<int>> clauses; int vars; bool expected; };
    std::vector<Test> tests;

    for (int n = 2; n <= 20; n++) {
        tests.push_back({"PHP_"+std::to_string(n), gen_php(n), n*(n-1), false});
    }

    tests.push_back({"K3_2col", gen_kcol(3,2), 6, false});
    tests.push_back({"K4_2col", gen_kcol(4,2), 8, false});
    tests.push_back({"K4_3col", gen_kcol(4,3), 12, false});
    tests.push_back({"K5_2col", gen_kcol(5,2), 10, false});
    tests.push_back({"K3_3col", gen_kcol(3,3), 9, true});
    tests.push_back({"K2_2col", gen_kcol(2,2), 4, true});

    int correct = 0;
    for (auto& t : tests) {
        GoldenFibonacciMirror solver(t.vars, t.clauses);
        auto start = std::chrono::high_resolution_clock::now();
        bool result = solver.solve();
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        bool ok = (result == t.expected);
        if (ok) correct++;
        double sn = sublinear_formula(t.vars);

        std::cout << "  " << std::left << std::setw(14) << t.name
                  << std::setw(8) << t.vars
                  << std::setw(10) << (t.expected ? "SAT" : "UNSAT")
                  << std::setw(10) << (result ? "SAT" : "UNSAT")
                  << std::setw(10) << solver.decisions
                  << std::setw(8) << solver.nodes_explored
                  << std::setw(8) << solver.propagations
                  << std::setw(8) << solver.conflicts
                  << std::setw(10) << std::fixed << std::setprecision(2) << sn
                  << "  " << (ok ? "OK" : "FAIL") << "  " << ms << "ms"
                  << "\n";
    }
    std::cout << "  " << std::string(90, '-') << "\n";
    std::cout << "  Correct: " << correct << "/" << tests.size() 
              << " (" << std::fixed << std::setprecision(1) << (100.0*correct/tests.size()) << "%)\n\n";

    return 0;
}
