#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

inline double FGG(double v) {
    double c = v;
    for (int d = 0; d < 3; d++)
        c = std::abs(c * ((d % 2 == 0) ? PHI * PSI : PSI * PHI));
    return c;
}

struct MultiDomainErasureSolver {
    int n_vars;
    std::vector<std::vector<int>> clauses;
    std::vector<int> assign, level;
    int dl;
    long long nodes = 0;
    long long erasures_1d = 0, erasures_2d = 0, erasures_3d = 0;

    MultiDomainErasureSolver(int n, const std::vector<std::vector<int>>& cls) 
        : n_vars(n), clauses(cls), dl(0) {
        assign.resize(n_vars + 1, 0);
        level.resize(n_vars + 1, -1);
    }

    int lit_value(int lit) {
        int var = abs(lit);
        if (assign[var] == 0) return 0;
        return (assign[var] == 1) ? (lit > 0 ? 1 : -1) : (lit > 0 ? -1 : 1);
    }

    bool clause_satisfied(const std::vector<int>& cl) {
        for (int lit : cl) if (lit_value(lit) == 1) return true;
        return false;
    }

    // Dimension 1: Clause satisfaction (φ-domain)
    double dim_clause() {
        int sat = 0;
        for (auto& cl : clauses) if (clause_satisfied(cl)) sat++;
        return FGG((double)sat / clauses.size());
    }

    // Dimension 2: Variable assignment density (ψ-domain)
    double dim_density() {
        int assigned = 0;
        for (int v = 1; v <= n_vars; v++) if (assign[v] != 0) assigned++;
        return FGG((double)assigned / n_vars);
    }

    // Dimension 3: Conflict potential (φ·ψ domain)
    double dim_conflict() {
        int potential_conflicts = 0;
        for (auto& cl : clauses) {
            int unassigned = 0;
            bool any_false = false;
            for (int lit : cl) {
                int val = lit_value(lit);
                if (val == 1) { any_false = false; break; }
                if (val == 0) unassigned++;
                if (val == -1) any_false = true;
            }
            if (any_false && unassigned <= 2) potential_conflicts++;
        }
        return FGG(1.0 - (double)potential_conflicts / clauses.size());
    }

    // Multi-dimensional erasure: combine all 3 domains
    double multi_signal() {
        double d1 = dim_clause();
        double d2 = dim_density();
        double d3 = dim_conflict();
        // Tensor product erasure: collapse across all dimensions
        return FGG(d1 * d2 * d3);
    }

    int multi_choose_var() {
        int best = 0;
        double best_signal = -1;

        for (int v = 1; v <= n_vars; v++) {
            if (assign[v] != 0) continue;

            assign[v] = 1;
            double s1 = multi_signal();
            assign[v] = 0;

            assign[v] = -1;
            double s2 = multi_signal();
            assign[v] = 0;

            double max_s = std::max(s1, s2);
            if (max_s > best_signal) { best_signal = max_s; best = v; }
        }

        // Track which dimension contributed to erasure
        if (best_signal > 0.9) {
            if (dim_clause() > 0.9) erasures_1d++;
            if (dim_density() > 0.9) erasures_2d++;
            if (dim_conflict() > 0.9) erasures_3d++;
        }

        return best;
    }

    bool bcp() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (auto& cl : clauses) {
                if (clause_satisfied(cl)) continue;
                int unassigned = 0, last_lit = 0;
                bool any_false = false;
                for (int lit : cl) {
                    int val = lit_value(lit);
                    if (val == 1) { any_false = false; break; }
                    if (val == 0) { unassigned++; last_lit = lit; }
                    if (val == -1) any_false = true;
                }
                if (clause_satisfied(cl)) continue;
                if (unassigned == 0 && any_false) return false;
                if (unassigned == 1 && !clause_satisfied(cl)) {
                    int var = abs(last_lit);
                    assign[var] = (last_lit > 0) ? 1 : -1;
                    level[var] = dl;
                    changed = true;
                }
            }
        }
        return true;
    }

    bool solve() {
        nodes++;
        if (!bcp()) return false;
        int var = multi_choose_var();
        if (var == 0) return true;
        dl++;
        assign[var] = 1; level[var] = dl;
        if (solve()) return true;
        assign[var] = -1; level[var] = dl;
        if (solve()) return true;
        assign[var] = 0; level[var] = -1; dl--;
        return false;
    }
};

std::pair<int, std::vector<std::vector<int>>> pigeonhole(int n) {
    int p = n + 1, h = n;
    int n_vars = p * h;
    std::vector<std::vector<int>> cls;
    for (int i = 0; i < p; i++) {
        std::vector<int> cl;
        for (int j = 0; j < h; j++) cl.push_back(i * h + j + 1);
        cls.push_back(cl);
    }
    for (int j = 0; j < h; j++)
        for (int i1 = 0; i1 < p; i1++)
            for (int i2 = i1 + 1; i2 < p; i2++)
                cls.push_back({-(i1 * h + j + 1), -(i2 * h + j + 1)});
    return {n_vars, cls};
}

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  MULTI-DIMENSIONAL RECURSIVE FRACTAL ERASURE\n";
    std::cout << "  3 Domains: Clause(φ) × Density(ψ) × Conflict(φ·ψ)\n";
    std::cout << "  Tensor product erasure: FGG(d1 × d2 × d3)\n";
    std::cout << "================================================================================\n\n";

    std::cout << std::left << std::setw(8) << "Problem"
              << std::setw(8) << "Vars"
              << std::setw(8) << "Nodes"
              << std::setw(10) << "1D(φ)"
              << std::setw(10) << "2D(ψ)"
              << std::setw(10) << "3D(φ·ψ)"
              << std::setw(10) << "Time(ms)"
              << "Status\n";
    std::cout << std::string(64, '-') << "\n";

    for (int n = 2; n <= 8; n++) {
        auto [n_vars, cls] = pigeonhole(n);
        auto start = std::chrono::steady_clock::now();
        MultiDomainErasureSolver solver(n_vars, cls);
        bool result = solver.solve();
        auto end = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << std::left << std::setw(8) << (std::to_string(n+1)+"p/"+std::to_string(n)+"h")
                  << std::setw(8) << n_vars
                  << std::setw(8) << solver.nodes
                  << std::setw(10) << solver.erasures_1d
                  << std::setw(10) << solver.erasures_2d
                  << std::setw(10) << solver.erasures_3d
                  << std::setw(10) << ms
                  << (result ? "SAT" : "UNSAT") << "\n";
    }

    std::cout << "\n  3D Erasure: All three domains collapse simultaneously.\n";
    std::cout << "  φ-domain: clause satisfaction signal\n";
    std::cout << "  ψ-domain: variable density signal\n";
    std::cout << "  φ·ψ domain: conflict proximity signal\n";
    std::cout << "================================================================================\n\n";
}
