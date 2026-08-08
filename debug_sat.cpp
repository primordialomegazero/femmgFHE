#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DebugSAT {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<double> observer;
    std::vector<double> observed;
    int operations;

    DebugSAT(int n, const std::vector<std::vector<int>>& cls)
        : n_vars(n), clauses(cls), operations(0) {
        observer.resize(n_vars, PHI);
        observed.resize(n_vars, PSI);
    }

    double golden_fib(int k) {
        if (k <= 0) return 0;
        if (k == 1 || k == 2) return 1;
        double a = 1, b = 1, c;
        for (int i = 3; i <= k; i++) { c = a + b; a = b; b = c; }
        return b;
    }

    void print_state(const std::string& label) {
        std::cout << "    [" << label << "]\n    ";
        for (int i = 0; i < n_vars; i++) {
            std::cout << "v" << i << "(" << std::fixed << std::setprecision(3) 
                      << observer[i] << "/" << observed[i] << ") ";
        }
        std::cout << "\n";
    }

    void fractal_diffusion() {
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
            if (fabs(sum) > 1e-12) { observer[i] /= sum; observed[i] /= sum; }
        }
    }

    void apply_clause_tension() {
        for (const auto& cl : clauses) {
            double max_obs = 0, max_obd = 0;
            for (int lit : cl) {
                int var = abs(lit) - 1;
                if (lit > 0) { if (observer[var] > max_obs) max_obs = observer[var]; }
                else { if (observed[var] > max_obd) max_obd = observed[var]; }
            }
            double sat_level = (max_obs + max_obd) / 2.0;
            double tension = 1.0 - sat_level;
            if (tension > 0.0) {
                double push = tension * (1.0 / PHI);
                for (int lit : cl) {
                    int var = abs(lit) - 1;
                    if (lit > 0) { observer[var] += push * (1.0 - observer[var]); }
                    else { observed[var] += push * (1.0 - observed[var]); }
                    double sum = observer[var] + observed[var];
                    if (fabs(sum) > 1e-12) { observer[var] /= sum; observed[var] /= sum; }
                }
            }
        }
    }

    void analyze() {
        std::cout << "\n  CLAUSE ANALYSIS:\n";
        for (int ci = 0; ci < (int)clauses.size(); ci++) {
            const auto& cl = clauses[ci];
            double max_obs = 0, max_obd = 0;
            std::cout << "  Clause " << ci << " [";
            for (int lit : cl) std::cout << lit << " ";
            std::cout << "] -> ";
            for (int lit : cl) {
                int var = abs(lit) - 1;
                if (lit > 0) {
                    std::cout << "obs" << var << "=" << std::fixed << std::setprecision(3) << observer[var] << " ";
                    if (observer[var] > max_obs) max_obs = observer[var];
                } else {
                    std::cout << "obd" << var << "=" << std::fixed << std::setprecision(3) << observed[var] << " ";
                    if (observed[var] > max_obd) max_obd = observed[var];
                }
            }
            double sat_level = (max_obs + max_obd) / 2.0;
            std::cout << "sat=" << std::fixed << std::setprecision(3) << sat_level;
            if (sat_level < 0.2) std::cout << " UNSAT";
            std::cout << "\n";
        }
    }

    bool detect() {
        print_state("INITIAL");

        for (int iter = 0; iter < n_vars; iter++) {
            fractal_diffusion();
            apply_clause_tension();
        }

        print_state("FINAL");
        analyze();

        for (const auto& cl : clauses) {
            double max_obs = 0, max_obd = 0;
            for (int lit : cl) {
                int var = abs(lit) - 1;
                if (lit > 0) { if (observer[var] > max_obs) max_obs = observer[var]; }
                else { if (observed[var] > max_obd) max_obd = observed[var]; }
            }
            double sat_level = (max_obs + max_obd) / 2.0;
            if (sat_level < 0.2) return false;
        }
        return true;
    }
};

int main() {
    std::cout << "\n========================================\n";
    std::cout << "K2_2col (SAT expected)\n";
    std::cout << "========================================\n";
    std::vector<std::vector<int>> k2 = {
        {1,2}, {3,4}, {-1,-3}, {-2,-4}
    };
    DebugSAT d1(4, k2);
    bool r1 = d1.detect();
    std::cout << "\nResult: " << (r1 ? "SAT" : "UNSAT") << " (expected SAT)\n\n";

    std::cout << "========================================\n";
    std::cout << "K3_3col (SAT expected)\n";
    std::cout << "========================================\n";
    std::vector<std::vector<int>> k33 = {
        {1,2,3}, {4,5,6}, {7,8,9},
        {-1,-4}, {-1,-7}, {-4,-7},
        {-2,-5}, {-2,-8}, {-5,-8},
        {-3,-6}, {-3,-9}, {-6,-9}
    };
    DebugSAT d2(9, k33);
    bool r2 = d2.detect();
    std::cout << "\nResult: " << (r2 ? "SAT" : "UNSAT") << " (expected SAT)\n\n";

    std::cout << "========================================\n";
    std::cout << "SAT_simple (SAT expected)\n";
    std::cout << "========================================\n";
    std::vector<std::vector<int>> sat3 = {{1, 2}, {-1, 3}, {-2, -3}};
    DebugSAT d3(3, sat3);
    bool r3 = d3.detect();
    std::cout << "\nResult: " << (r3 ? "SAT" : "UNSAT") << " (expected SAT)\n";

    return 0;
}
