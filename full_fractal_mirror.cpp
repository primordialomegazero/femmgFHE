#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct FullFractalMirror {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<double> observer;
    std::vector<double> observed;
    std::vector<int> clause_weight;
    int operations;

    FullFractalMirror(int n, const std::vector<std::vector<int>>& cls)
        : n_vars(n), clauses(cls), operations(0) {
        observer.resize(n_vars, PHI);
        observed.resize(n_vars, PSI);
        clause_weight.resize(cls.size(), 0);
    }

    double golden_fib(int k) {
        if (k <= 0) return 0;
        if (k == 1 || k == 2) return 1;
        double a = 1, b = 1, c;
        for (int i = 3; i <= k; i++) {
            c = a + b;
            a = b;
            b = c;
        }
        return b;
    }

    void apply_clause_tension() {
        for (int ci = 0; ci < (int)clauses.size(); ci++) {
            const auto& cl = clauses[ci];
            
            double max_obs = 0, max_obd = 0;
            double sum_obs = 0, sum_obd = 0;
            int count = 0;

            for (int lit : cl) {
                int var = abs(lit) - 1;
                if (lit > 0) {
                    sum_obs += observer[var];
                    if (observer[var] > max_obs) max_obs = observer[var];
                } else {
                    sum_obd += observed[var];
                    if (observed[var] > max_obd) max_obd = observed[var];
                }
                count++;
            }

            double clause_sat = (max_obs + max_obd) / 2.0;
            double clause_conflict = 1.0 - clause_sat;
            clause_weight[ci] = clause_conflict;

            if (clause_conflict > 0.5) {
                double push = clause_conflict * (1.0 / PHI);
                for (int lit : cl) {
                    int var = abs(lit) - 1;
                    if (lit > 0) {
                        observer[var] += push * observed[var];
                    } else {
                        observed[var] += push * observer[var];
                    }
                    double sum = observer[var] + observed[var];
                    if (fabs(sum) > 1e-12) {
                        observer[var] /= sum;
                        observed[var] /= sum;
                    }
                }
            }
        }
    }

    void fractal_diffusion(int iterations) {
        for (int iter = 0; iter < iterations; iter++) {
            std::vector<double> new_obs(n_vars, 0);
            std::vector<double> new_obd(n_vars, 0);
            std::vector<double> weights(n_vars, 0);

            for (int i = 0; i < n_vars; i++) {
                for (int j = 0; j < n_vars; j++) {
                    int dist = abs(i - j);
                    double fib = golden_fib(dist + 1);
                    double w = 1.0 / (1.0 + fib / PHI);
                    
                    new_obs[i] += observer[j] * w;
                    new_obd[i] += observed[j] * w;
                    weights[i] += w;
                }
                operations++;
            }

            for (int i = 0; i < n_vars; i++) {
                if (weights[i] > 0) {
                    observer[i] = new_obs[i] / weights[i];
                    observed[i] = new_obd[i] / weights[i];
                }
                double sum = observer[i] + observed[i];
                if (fabs(sum) > 1e-12) {
                    observer[i] /= sum;
                    observed[i] /= sum;
                }
            }

            apply_clause_tension();
        }
    }

    bool detect() {
        for (const auto& cl : clauses) {
            if (cl.size() == 1) {
                int lit = cl[0];
                int var = abs(lit) - 1;
                if (lit > 0) {
                    observer[var] = 1.0;
                    observed[var] = 0.0;
                } else {
                    observer[var] = 0.0;
                    observed[var] = 1.0;
                }
            }
        }

        fractal_diffusion(n_vars);

        double total_harmony = 0, total_conflict = 0;
        for (int i = 0; i < n_vars; i++) {
            total_harmony += observer[i] + observed[i];
            total_conflict += observer[i] * observed[i];
            operations++;
        }
        total_harmony /= n_vars;
        total_conflict /= n_vars;

        int unsat_clauses = 0;
        for (int ci = 0; ci < (int)clauses.size(); ci++) {
            if (clause_weight[ci] > 0.8) unsat_clauses++;
            operations++;
        }

        if (unsat_clauses > 0) return false;
        if (total_conflict < -0.05) return false;
        if (total_conflict > 0.24 && fabs(total_harmony - 1.0) < 0.01) return false;

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
    std::cout << "  FULL FRACTAL MIRROR\n";
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

    for (int n = 2; n <= 15; n++) {
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
        FullFractalMirror detector(t.vars, t.clauses);
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
