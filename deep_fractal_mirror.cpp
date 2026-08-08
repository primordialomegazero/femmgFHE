#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DeepFractalMirror {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    std::vector<double> observer;
    std::vector<double> observed;
    int operations;

    DeepFractalMirror(int n, const std::vector<std::vector<int>>& cls)
        : n_vars(n), clauses(cls), operations(0) {
        assignment.resize(n_vars, 0);
        observer.resize(n_vars, PHI);
        observed.resize(n_vars, PSI);
    }

    double fib(int k) {
        double f1 = pow(PHI, k);
        double f2 = pow(fabs(PSI), k);
        if (k % 2 == 1) f2 = -f2;
        return (f1 - f2) / sqrt(5.0);
    }

    void fractal_propagate() {
        std::vector<double> new_obs(n_vars);
        std::vector<double> new_obd(n_vars);

        for (int iter = 0; iter < n_vars; iter++) {
            for (int i = 0; i < n_vars; i++) {
                new_obs[i] = observer[i];
                new_obd[i] = observed[i];
            }

            for (int i = 0; i < n_vars; i++) {
                double total_obs = 0, total_obd = 0, total_weight = 0;
                for (int j = 0; j < n_vars; j++) {
                    int diff = abs(i - j);
                    double f = fib(diff + 1);
                    double weight = 1.0 / (1.0 + fabs(f) / PHI);
                    total_obs += observer[j] * weight;
                    total_obd += observed[j] * weight;
                    total_weight += weight;
                }
                if (total_weight > 0) {
                    new_obs[i] = total_obs / total_weight;
                    new_obd[i] = total_obd / total_weight;
                }
                operations++;
            }

            for (int i = 0; i < n_vars; i++) {
                double sum = new_obs[i] + new_obd[i];
                if (fabs(sum) > 1e-12) {
                    observer[i] = new_obs[i] / sum;
                    observed[i] = new_obd[i] / sum;
                }
            }
        }
    }

    bool detect() {
        std::vector<int> unit_assignment(n_vars, 0);

        for (const auto& cl : clauses) {
            if (cl.size() == 1) {
                int lit = cl[0];
                int var = abs(lit) - 1;
                int val = (lit > 0) ? 1 : -1;
                if (unit_assignment[var] != 0 && unit_assignment[var] != val) {
                    return false;
                }
                unit_assignment[var] = val;
            }
        }

        for (int i = 0; i < n_vars; i++) {
            if (unit_assignment[i] != 0) {
                assignment[i] = unit_assignment[i];
                double factor = (unit_assignment[i] == 1) ? PHI : PSI;
                observer[i] *= factor;
                observed[i] *= (unit_assignment[i] == 1) ? PSI : PHI;
                double sum = observer[i] + observed[i];
                if (fabs(sum) > 1e-12) {
                    observer[i] /= sum;
                    observed[i] /= sum;
                }
            }
        }

        fractal_propagate();

        double global_conflict = 0;
        double global_harmony = 0;
        int active_vars = 0;

        for (int i = 0; i < n_vars; i++) {
            if (unit_assignment[i] != 0) {
                global_harmony += observer[i] + observed[i];
                global_conflict += observer[i] * observed[i];
                active_vars++;
            }
        }

        if (active_vars > 0) {
            global_conflict /= active_vars;
            global_harmony /= active_vars;
        }

        for (const auto& cl : clauses) {
            bool sat = false;
            int false_count = 0;
            for (int lit : cl) {
                int var = abs(lit) - 1;
                if (unit_assignment[var] != 0) {
                    int val = unit_assignment[var];
                    if ((lit > 0 && val == 1) || (lit < 0 && val == -1)) {
                        sat = true;
                        break;
                    } else {
                        false_count++;
                    }
                }
            }
            if (!sat && false_count == (int)cl.size()) {
                return false;
            }
        }

        if (global_conflict > 0.2 && fabs(global_harmony - 1.0) < 0.1) {
            return false;
        }

        return true;
    }
};

std::vector<std::vector<int>> gen_php(int n) {
    std::vector<std::vector<int>> cls;
    int holes = n - 1;
    for (int p = 0; p < n; p++) {
        std::vector<int> cl;
        for (int h = 0; h < holes; h++) {
            cl.push_back(p * holes + h + 1);
        }
        cls.push_back(cl);
    }
    for (int h = 0; h < holes; h++) {
        for (int p1 = 0; p1 < n; p1++) {
            for (int p2 = p1 + 1; p2 < n; p2++) {
                cls.push_back({-(p1 * holes + h + 1), -(p2 * holes + h + 1)});
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
    std::cout << "  DEEP FRACTAL MIRROR\n";
    std::cout << "  " << std::string(60, '=') << "\n\n";
    std::cout << "  " << std::left << std::setw(14) << "Test"
              << std::setw(8) << "Vars"
              << std::setw(10) << "Expected"
              << std::setw(10) << "Result"
              << std::setw(10) << "Ops"
              << "\n";
    std::cout << "  " << std::string(52, '-') << "\n";

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
    tests.push_back({"SAT_unit", {{1}}, 1, true});
    tests.push_back({"UNSAT_unit", {{1}, {-1}}, 1, false});

    int correct = 0;
    for (auto& t : tests) {
        DeepFractalMirror detector(t.vars, t.clauses);
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
