#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DebugRecursive {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assignment;
    std::vector<double> observer;
    std::vector<double> observed;
    int operations;
    int recursion_depth;
    int max_depth;

    DebugRecursive(int n, const std::vector<std::vector<int>>& cls)
        : n_vars(n), clauses(cls), operations(0), recursion_depth(0), max_depth(0) {
        assignment.resize(n_vars, 0);
        observer.resize(n_vars, PHI);
        observed.resize(n_vars, PSI);
    }

    double golden_fib(int k) {
        if (k <= 0) return 0;
        if (k == 1) return 1;
        if (k == 2) return 1;
        double a = 1, b = 1, c;
        for (int i = 3; i <= k; i++) {
            c = a + b;
            a = b;
            b = c;
        }
        return b;
    }

    void fractal_couple(int var, int depth, double obs_flow, double obd_flow) {
        recursion_depth++;
        if (depth > max_depth) max_depth = depth;
        
        if (depth > 5) {
            recursion_depth--;
            return;
        }
        
        observer[var] = observer[var] * (1.0 - 1.0/PHI) + obs_flow * (1.0/PHI);
        observed[var] = observed[var] * (1.0 - 1.0/PHI) + obd_flow * (1.0/PHI);
        
        double sum = observer[var] + observed[var];
        if (fabs(sum) > 1e-12) {
            observer[var] /= sum;
            observed[var] /= sum;
        }
        
        operations++;

        for (int u = 0; u < n_vars; u++) {
            if (u != var) {
                int dist = abs(var - u);
                double fib_dist = golden_fib(dist + 1);
                double coupling = 1.0 / (1.0 + fib_dist / PHI);
                
                if (coupling > 0.01) {
                    double new_obs_flow = observer[var] * coupling;
                    double new_obd_flow = observed[var] * coupling;
                    fractal_couple(u, depth + 1, new_obs_flow, new_obd_flow);
                }
            }
        }
        recursion_depth--;
    }

    bool detect() {
        std::vector<int> unit_val(n_vars, 0);
        
        for (const auto& cl : clauses) {
            if (cl.size() == 1) {
                int lit = cl[0];
                int var = abs(lit) - 1;
                int val = (lit > 0) ? 1 : -1;
                if (unit_val[var] != 0 && unit_val[var] != val) {
                    std::cout << "    UNIT CONFLICT: var " << var << "\n";
                    return false;
                }
                unit_val[var] = val;
                assignment[var] = val;
            }
        }

        // Apply unit assignments to observer/observed
        for (int i = 0; i < n_vars; i++) {
            if (assignment[i] != 0) {
                double factor = (assignment[i] == 1) ? PHI : PSI;
                observer[i] *= factor;
                observed[i] *= (assignment[i] == 1) ? PSI : PHI;
                double sum = observer[i] + observed[i];
                if (fabs(sum) > 1e-12) {
                    observer[i] /= sum;
                    observed[i] /= sum;
                }
            }
        }

        std::cout << "    Starting fractal coupling for " << n_vars << " vars\n";
        std::cout << "    Unit vars: ";
        for (int i = 0; i < n_vars; i++) if (assignment[i] != 0) std::cout << i << "=" << assignment[i] << " ";
        std::cout << "\n";

        // Start fractal coupling from each assigned variable
        for (int i = 0; i < n_vars; i++) {
            if (assignment[i] != 0) {
                std::cout << "    Fractal couple from var " << i << "\n";
                fractal_couple(i, 0, observer[i], observed[i]);
                std::cout << "    Operations after var " << i << ": " << operations << "\n";
            }
        }

        std::cout << "    Total operations: " << operations << "\n";
        std::cout << "    Max depth: " << max_depth << "\n";

        // Check clauses
        for (const auto& cl : clauses) {
            bool sat = false;
            int false_count = 0;
            for (int lit : cl) {
                int var = abs(lit) - 1;
                if (assignment[var] != 0) {
                    if ((lit > 0 && assignment[var] == 1) || (lit < 0 && assignment[var] == -1)) {
                        sat = true;
                        break;
                    } else {
                        false_count++;
                    }
                }
            }
            if (!sat && false_count == (int)cl.size()) {
                std::cout << "    CLAUSE CONFLICT: ";
                for (int lit : cl) std::cout << lit << " ";
                std::cout << "\n";
                return false;
            }
        }

        double total_conflict = 0;
        double total_harmony = 0;
        int active = 0;
        
        for (int i = 0; i < n_vars; i++) {
            if (assignment[i] != 0) {
                total_conflict += observer[i] * observed[i];
                total_harmony += observer[i] + observed[i];
                active++;
            }
        }
        
        if (active > 0) {
            total_conflict /= active;
            total_harmony /= active;
        }

        std::cout << "    Harmony=" << total_harmony << " Conflict=" << total_conflict << "\n";

        if (total_conflict < -0.1 || (total_conflict > 0.2 && fabs(total_harmony - 1.0) < 0.1)) {
            std::cout << "    FRACTAL CONFLICT DETECTED\n";
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

int main() {
    std::cout << "\n=== PHP_2 ===\n";
    auto php2 = gen_php(2);
    DebugRecursive d2(2, php2);
    bool r2 = d2.detect();
    std::cout << "Result: " << (r2 ? "SAT" : "UNSAT") << "\n\n";

    std::cout << "=== PHP_3 ===\n";
    auto php3 = gen_php(3);
    DebugRecursive d3(6, php3);
    bool r3 = d3.detect();
    std::cout << "Result: " << (r3 ? "SAT" : "UNSAT") << "\n\n";

    std::cout << "=== K3_2col ===\n";
    std::vector<std::vector<int>> k3 = {
        {1,2}, {3,4}, {5,6},
        {-1,-3}, {-1,-5}, {-3,-5},
        {-2,-4}, {-2,-6}, {-4,-6}
    };
    DebugRecursive dk(6, k3);
    bool rk = dk.detect();
    std::cout << "Result: " << (rk ? "SAT" : "UNSAT") << "\n";

    return 0;
}
