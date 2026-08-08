#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DebugTwoWay {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<double> observer;
    std::vector<double> observed;
    int operations;

    DebugTwoWay(int n, const std::vector<std::vector<int>>& cls)
        : n_vars(n), clauses(cls), operations(0) {
        observer.resize(n_vars, PHI);
        observed.resize(n_vars, PSI);
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

    void print_state(const std::string& label) {
        std::cout << "    [" << label << "]\n";
        std::cout << "    ";
        for (int i = 0; i < n_vars; i++) {
            std::cout << "v" << i << "(" << std::fixed << std::setprecision(3) 
                      << observer[i] << "/" << observed[i] << ") ";
        }
        std::cout << "\n";
        double h = 0, c = 0;
        for (int i = 0; i < n_vars; i++) {
            h += observer[i] + observed[i];
            c += observer[i] * observed[i];
        }
        h /= n_vars; c /= n_vars;
        std::cout << "    Harmony=" << std::fixed << std::setprecision(6) << h 
                  << " Conflict=" << std::fixed << std::setprecision(6) << c << "\n\n";
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
            operations++;
        }
    }

    void apply_clause_tension() {
        for (int ci = 0; ci < (int)clauses.size(); ci++) {
            const auto& cl = clauses[ci];
            double max_obs = 0, max_obd = 0;
            for (int lit : cl) {
                int var = abs(lit) - 1;
                if (lit > 0) {
                    if (observer[var] > max_obs) max_obs = observer[var];
                } else {
                    if (observed[var] > max_obd) max_obd = observed[var];
                }
            }
            double clause_sat = (max_obs + max_obd) / 2.0;
            double tension = 1.0 - clause_sat;

            if (tension > 0.5) {
                for (int lit : cl) {
                    int var = abs(lit) - 1;
                    double push = tension * (1.0 / PHI);
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

    bool detect() {
        print_state("INITIAL");

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
        print_state("AFTER UNIT");

        for (int iter = 0; iter < std::min(3, n_vars); iter++) {
            fractal_diffusion(1);
            print_state("AFTER DIFFUSION " + std::to_string(iter+1));
            apply_clause_tension();
            print_state("AFTER TENSION " + std::to_string(iter+1));
        }

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
            const auto& cl = clauses[ci];
            double max_obs = 0, max_obd = 0;
            for (int lit : cl) {
                int var = abs(lit) - 1;
                if (lit > 0) {
                    if (observer[var] > max_obs) max_obs = observer[var];
                } else {
                    if (observed[var] > max_obd) max_obd = observed[var];
                }
            }
            double sat_level = (max_obs + max_obd) / 2.0;
            if (sat_level < 0.2) {
                unsat_clauses++;
                std::cout << "    UNSAT clause " << ci << ": sat_level=" << sat_level << " -> ";
                for (int lit : cl) std::cout << lit << " ";
                std::cout << "\n";
            }
        }

        std::cout << "    Unsat clauses: " << unsat_clauses << "\n";
        std::cout << "    Final Harmony=" << total_harmony << " Conflict=" << total_conflict << "\n";

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

int main() {
    std::cout << "\n========================================\n";
    std::cout << "PHP_2\n";
    std::cout << "========================================\n";
    auto php2 = gen_php(2);
    DebugTwoWay d2(2, php2);
    bool r2 = d2.detect();
    std::cout << "Result: " << (r2 ? "SAT" : "UNSAT") << "\n\n";

    std::cout << "========================================\n";
    std::cout << "PHP_3\n";
    std::cout << "========================================\n";
    auto php3 = gen_php(3);
    DebugTwoWay d3(6, php3);
    bool r3 = d3.detect();
    std::cout << "Result: " << (r3 ? "SAT" : "UNSAT") << "\n\n";

    std::cout << "========================================\n";
    std::cout << "K3_2col\n";
    std::cout << "========================================\n";
    std::vector<std::vector<int>> k3 = {
        {1,2}, {3,4}, {5,6},
        {-1,-3}, {-1,-5}, {-3,-5},
        {-2,-4}, {-2,-6}, {-4,-6}
    };
    DebugTwoWay dk(6, k3);
    bool rk = dk.detect();
    std::cout << "Result: " << (rk ? "SAT" : "UNSAT") << "\n";

    return 0;
}
