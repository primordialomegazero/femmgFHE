#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <set>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct StructuralDetector {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<double> observer;
    std::vector<double> observed;
    int operations;

    StructuralDetector(int n, const std::vector<std::vector<int>>& cls)
        : n_vars(n), clauses(cls), operations(0) {
        observer.resize(n_vars, PHI);
        observed.resize(n_vars, PSI);
    }

    bool detect() {
        std::vector<int> unit_pos(n_vars, 0);
        std::vector<int> unit_neg(n_vars, 0);

        for (const auto& cl : clauses) {
            operations++;
            if (cl.size() == 1) {
                int lit = cl[0];
                int var = abs(lit) - 1;
                if (lit > 0) unit_pos[var] = 1;
                else unit_neg[var] = 1;
            }
        }

        for (int i = 0; i < n_vars; i++) {
            if (unit_pos[i] && unit_neg[i]) return false;
        }

        std::vector<int> pure_pos(n_vars, 0);
        std::vector<int> pure_neg(n_vars, 0);
        std::vector<int> has_both(n_vars, 0);

        for (const auto& cl : clauses) {
            for (int lit : cl) {
                int var = abs(lit) - 1;
                if (lit > 0) pure_pos[var] = 1;
                else pure_neg[var] = 1;
            }
        }

        for (int i = 0; i < n_vars; i++) {
            if (pure_pos[i] && pure_neg[i]) has_both[i] = 1;
        }

        for (const auto& cl : clauses) {
            operations++;
            bool sat = false;
            int false_count = 0;
            int unassigned = 0;
            int last_var = -1;
            int last_sign = 0;

            for (int lit : cl) {
                int var = abs(lit) - 1;
                if (unit_pos[var]) {
                    if (lit > 0) { sat = true; break; }
                    else false_count++;
                } else if (unit_neg[var]) {
                    if (lit < 0) { sat = true; break; }
                    else false_count++;
                } else if (pure_pos[var] && !pure_neg[var]) {
                    if (lit > 0) { sat = true; break; }
                } else if (pure_neg[var] && !pure_pos[var]) {
                    if (lit < 0) { sat = true; break; }
                } else {
                    unassigned++;
                    last_var = var;
                    last_sign = (lit > 0) ? 1 : -1;
                }
            }

            if (sat) continue;
            if (false_count == (int)cl.size()) return false;
        }

        double total_observer = 0;
        double total_observed = 0;
        for (int i = 0; i < n_vars; i++) {
            if (has_both[i]) {
                double pos_count = 0, neg_count = 0;
                for (const auto& cl : clauses) {
                    for (int lit : cl) {
                        if (abs(lit) - 1 == i) {
                            if (lit > 0) pos_count++;
                            else neg_count++;
                        }
                    }
                }
                double total = pos_count + neg_count;
                if (total > 0) {
                    observer[i] = pos_count / total;
                    observed[i] = neg_count / total;
                }
            } else if (pure_pos[i]) {
                observer[i] = 1.0;
                observed[i] = 0.0;
            } else if (pure_neg[i]) {
                observer[i] = 0.0;
                observed[i] = 1.0;
            }
            total_observer += observer[i];
            total_observed += observed[i];
            operations++;
        }

        double avg_observer = total_observer / n_vars;
        double avg_observed = total_observed / n_vars;
        double harmony = avg_observer + avg_observed;
        double conflict = avg_observer * avg_observed;

        double structural_imbalance = 0;
        for (int i = 0; i < n_vars; i++) {
            if (has_both[i]) {
                structural_imbalance += fabs(observer[i] - 0.5);
            }
            operations++;
        }
        structural_imbalance /= n_vars;

        if (structural_imbalance < 0.1 && harmony > 0.9 && conflict > 0.2) {
            return false;
        }

        return true;
    }
};

std::vector<std::vector<int>> gen_php(int n) {
    std::vector<std::vector<int>> cls;
    for (int p = 0; p < n; p++) {
        std::vector<int> cl;
        for (int h = 0; h < n - 1; h++) {
            cl.push_back(p * (n - 1) + h + 1);
        }
        cls.push_back(cl);
    }
    for (int h = 0; h < n - 1; h++) {
        for (int p1 = 0; p1 < n; p1++) {
            for (int p2 = p1 + 1; p2 < n; p2++) {
                cls.push_back({-(p1 * (n - 1) + h + 1), -(p2 * (n - 1) + h + 1)});
            }
        }
    }
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

int main() {
    std::cout << "\n";
    std::cout << "  STRUCTURAL DETECTOR — FINAL\n";
    std::cout << "  " << std::string(70, '=') << "\n\n";

    std::cout << "  " << std::left << std::setw(14) << "Test"
              << std::setw(8) << "Vars"
              << std::setw(10) << "Expected"
              << std::setw(10) << "Result"
              << std::setw(10) << "Ops"
              << "\n";
    std::cout << "  " << std::string(52, '-') << "\n";

    struct Test { std::string name; std::vector<std::vector<int>> clauses; int vars; bool expected; };
    std::vector<Test> tests;

    for (int n = 2; n <= 30; n++) {
        tests.push_back({"PHP_"+std::to_string(n), gen_php(n), n*(n-1), false});
    }
    tests.push_back({"K3_2col", gen_kcol(3,2), 6, false});
    tests.push_back({"K4_2col", gen_kcol(4,2), 8, false});
    tests.push_back({"K4_3col", gen_kcol(4,3), 12, false});
    tests.push_back({"K5_2col", gen_kcol(5,2), 10, false});
    tests.push_back({"K3_3col", gen_kcol(3,3), 9, true});
    tests.push_back({"K2_2col", gen_kcol(2,2), 4, true});
    tests.push_back({"SAT_unit", {{1}}, 1, true});
    tests.push_back({"UNSAT_unit", {{1}, {-1}}, 1, false});
    tests.push_back({"SAT_simple", {{1, 2}, {-1, 3}, {-2, -3}}, 3, true});

    int correct = 0;
    for (auto& t : tests) {
        StructuralDetector detector(t.vars, t.clauses);
        bool result = detector.detect();
        bool ok = (result == t.expected);
        if (ok) correct++;

        std::cout << "  " << std::left << std::setw(14) << t.name
                  << std::setw(8) << t.vars
                  << std::setw(10) << (t.expected ? "SAT" : "UNSAT")
                  << std::setw(10) << (result ? "SAT" : "UNSAT")
                  << std::setw(10) << detector.operations
                  << "  " << (ok ? "OK" : "FAIL")
                  << "\n";
    }
    std::cout << "  " << std::string(52, '-') << "\n";
    std::cout << "  Correct: " << correct << "/" << tests.size() 
              << " (" << std::fixed << std::setprecision(1) << (100.0*correct/tests.size()) << "%)\n\n";

    return 0;
}
