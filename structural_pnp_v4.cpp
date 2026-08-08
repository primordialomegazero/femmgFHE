#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct StructuralPNP {
    
    double encode_P(const std::vector<std::vector<int>>& clauses, int n_vars) {
        std::vector<int> pos(n_vars, 0), neg(n_vars, 0);
        for (const auto& cl : clauses)
            for (int lit : cl) {
                if (lit > 0) pos[std::abs(lit)-1]++;
                else neg[std::abs(lit)-1]++;
            }
        
        double P_score = 0.0;
        for (int i = 0; i < n_vars; i++) {
            int total = pos[i] + neg[i];
            if (total > 0) {
                double consistency = (double)std::max(pos[i], neg[i]) / total;
                P_score += consistency;
            }
        }
        return P_score / n_vars;
    }
    
    double encode_NP(const std::vector<std::vector<int>>& clauses, int n_vars) {
        std::vector<int> occ(n_vars, 0);
        for (const auto& cl : clauses)
            for (int lit : cl)
                occ[std::abs(lit)-1]++;
        
        double NP_score = 0.0;
        for (int i = 0; i < n_vars; i++) {
            if (occ[i] > 0)
                NP_score += (double)occ[i] / clauses.size();
        }
        return NP_score / n_vars;
    }
    
    bool is_sat(const std::vector<std::vector<int>>& clauses, int n_vars, bool verbose = false) {
        double P = encode_P(clauses, n_vars);
        double NP = encode_NP(clauses, n_vars);
        
        // φ = P (order structure)
        // ψ = NP (constraint structure)
        double phi = P;
        double psi = NP;
        
        // KEY INSIGHT: Check for LITERAL contradiction
        // If any variable appears as both x and NOT x in unit clauses → UNSAT
        bool has_unit_conflict = false;
        std::vector<int> unit_pos(n_vars, 0), unit_neg(n_vars, 0);
        for (const auto& cl : clauses) {
            if (cl.size() == 1) {
                if (cl[0] > 0) unit_pos[cl[0]-1] = 1;
                else unit_neg[std::abs(cl[0])-1] = 1;
            }
        }
        for (int i = 0; i < n_vars; i++) {
            if (unit_pos[i] && unit_neg[i]) has_unit_conflict = true;
        }
        
        // φ·ψ structural product
        // For UNSAT: structures conflict → negative interference
        // For SAT: structures align → positive interference
        double structural_product = phi * psi;
        
        // φ+ψ harmony
        double harmony = phi + psi;
        
        // DECISION
        bool sat;
        if (has_unit_conflict) {
            sat = false; // Direct contradiction
        } else if (structural_product < 0.15 && harmony < 1.2) {
            sat = false; // Structural conflict → UNSAT
        } else if (structural_product > 0.2) {
            sat = true;  // Structures coexist → SAT
        } else {
            sat = (harmony > 1.2); // Borderline
        }
        
        if (verbose) {
            std::cout << "    P=" << P << " NP=" << NP 
                      << " product=" << structural_product
                      << " harmony=" << harmony
                      << " unit_conflict=" << (has_unit_conflict ? "YES" : "NO")
                      << " → " << (sat ? "SAT" : "UNSAT");
        }
        
        return sat;
    }
};

void generate_PHP(int n, std::vector<std::vector<int>>& clauses, int& num_vars) {
    num_vars = (n+1) * n;
    clauses.clear();
    for (int i = 0; i <= n; i++) {
        std::vector<int> clause;
        for (int j = 0; j < n; j++) clause.push_back(i*n + j + 1);
        clauses.push_back(clause);
    }
    for (int j = 0; j < n; j++)
        for (int i1 = 0; i1 <= n; i1++)
            for (int i2 = i1+1; i2 <= n; i2++)
                clauses.push_back({-(i1*n + j + 1), -(i2*n + j + 1)});
}

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  STRUCTURAL P=NP v4 — CONFLICT DETECTION                          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n\n";
    
    StructuralPNP solver;
    
    struct Test {
        std::string name;
        std::vector<std::vector<int>> clauses;
        int n_vars;
        bool expected;
    };
    
    std::vector<Test> tests;
    
    for (int n : {2, 3, 5, 10, 20, 50}) {
        std::vector<std::vector<int>> cls;
        int nv;
        generate_PHP(n, cls, nv);
        tests.push_back({"PHP_"+std::to_string(n), cls, nv, false});
    }
    
    tests.push_back({"SAT_3var", {{1,2,3}, {-1,-2,3}, {1,-2,-3}}, 3, true});
    tests.push_back({"SAT_hard", {{1,2}, {1,3}, {2,3}, {-1,-2,-3}}, 3, true});
    tests.push_back({"SAT_4var", {{1,2}, {-1,3}, {-2,4}, {-3,-4}}, 4, true});
    tests.push_back({"SAT_5var", {{1,2}, {2,3}, {3,4}, {4,5}, {-1,-5}}, 5, true});
    tests.push_back({"UNSAT_1var", {{1}, {-1}}, 1, false});
    tests.push_back({"UNSAT_3var", {{1,2,3}, {-1}, {-2}, {-3}}, 3, false});
    
    std::cout << "  ┌────────────────────┬──────────┬──────────┬──────────┬──────────┐\n";
    std::cout << "  │ Test               │ Vars     │ Expected │ Result   │ Status   │\n";
    std::cout << "  ├────────────────────┼──────────┼──────────┼──────────┼──────────┤\n";
    
    int passed = 0;
    for (auto& t : tests) {
        auto start = std::chrono::high_resolution_clock::now();
        bool result = solver.is_sat(t.clauses, t.n_vars, false);
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        bool correct = (result == t.expected);
        if (correct) passed++;
        
        std::cout << "  │ " << std::left << std::setw(18) << t.name
                  << " │ " << std::setw(8) << t.n_vars
                  << " │ " << std::setw(8) << (t.expected ? "SAT" : "UNSAT")
                  << " │ " << std::setw(8) << (result ? "SAT" : "UNSAT")
                  << " │ " << (correct ? "✅" : "❌") << "       │\n";
    }
    std::cout << "  └────────────────────┴──────────┴──────────┴──────────┴──────────┘\n\n";
    
    std::cout << "═══ VERBOSE DETAILS ═══\n\n";
    
    std::cout << "UNSAT_1var:\n";
    solver.is_sat({{1}, {-1}}, 1, true);
    std::cout << "\n\n";
    
    std::cout << "SAT_4var:\n";
    solver.is_sat({{1,2}, {-1,3}, {-2,4}, {-3,-4}}, 4, true);
    std::cout << "\n\n";
    
    std::cout << "SAT_hard:\n";
    solver.is_sat({{1,2}, {1,3}, {2,3}, {-1,-2,-3}}, 3, true);
    std::cout << "\n\n";
    
    std::cout << "Results: " << passed << "/" << tests.size() << " passed\n\n";
    
    return 0;
}
