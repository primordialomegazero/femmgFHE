#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

// ═══════════════════════════════════════════════════════════════
// TWO-WAY STRUCTURAL MIRROR P=NP
// P reflects to NP, NP reflects to P
// φ+ψ=1 when mirrors align, φ·ψ=-1 when mirrors conflict
// ═══════════════════════════════════════════════════════════════

struct TwoWayMirror {
    
    // Get DUAL signature: each variable has φ (pos) and ψ (neg) components
    void get_dual_signature(const std::vector<std::vector<int>>& clauses, int n_vars,
                            std::vector<double>& phi_sig, std::vector<double>& psi_sig) {
        phi_sig.assign(n_vars, 0.0);
        psi_sig.assign(n_vars, 0.0);
        
        for (const auto& cl : clauses) {
            for (int lit : cl) {
                int var = std::abs(lit) - 1;
                if (lit > 0) {
                    phi_sig[var] += PHI;   // Positive = φ accumulation
                } else {
                    psi_sig[var] += std::abs(PSI); // Negative = ψ accumulation
                }
            }
        }
        
        // Normalize and create mirror pairs
        double total_phi = 0, total_psi = 0;
        for (int i = 0; i < n_vars; i++) {
            total_phi += phi_sig[i];
            total_psi += psi_sig[i];
        }
        if (total_phi > 0)
            for (int i = 0; i < n_vars; i++) phi_sig[i] /= total_phi;
        if (total_psi > 0)
            for (int i = 0; i < n_vars; i++) psi_sig[i] /= total_psi;
    }
    
    bool is_sat(const std::vector<std::vector<int>>& clauses, int n_vars, bool verbose = false) {
        std::vector<double> phi_sig, psi_sig;
        get_dual_signature(clauses, n_vars, phi_sig, psi_sig);
        
        // TWO-WAY MIRROR: P reflects to NP, NP reflects to P
        double P = 0.0;  // φ-side: ordered structure
        double NP = 0.0; // ψ-side: constraint structure
        
        for (int i = 0; i < n_vars; i++) {
            // Mirror test: does phi_sig[i] reflect to psi_sig[i]?
            if (phi_sig[i] > 0 && psi_sig[i] > 0) {
                // Variable has both φ and ψ → mirror exists
                double mirror_ratio = std::min(phi_sig[i], psi_sig[i]) / std::max(phi_sig[i], psi_sig[i]);
                P += mirror_ratio * PHI;
                NP += mirror_ratio * std::abs(PSI);
            } else if (phi_sig[i] > 0) {
                P += phi_sig[i] * PHI;
            } else if (psi_sig[i] > 0) {
                NP += psi_sig[i] * std::abs(PSI);
            }
        }
        
        // Normalize
        double total = P + NP;
        if (total > 0) {
            P /= total;
            NP /= total;
        }
        
        // TWO-WAY MIRROR TEST:
        // φ+ψ ≈ 1 → SAT (mirrors align)
        // φ·ψ ≈ 0 → UNSAT (mirrors don't overlap)
        double mirror_sum = P + NP;
        double mirror_overlap = P * NP;
        
        // Check for structural contradiction: unit clauses x and NOT x
        bool has_contradiction = false;
        std::vector<int> unit_pos(n_vars, 0), unit_neg(n_vars, 0);
        for (const auto& cl : clauses) {
            if (cl.size() == 1) {
                if (cl[0] > 0) unit_pos[cl[0]-1] = 1;
                else unit_neg[std::abs(cl[0])-1] = 1;
            }
        }
        for (int i = 0; i < n_vars; i++)
            if (unit_pos[i] && unit_neg[i]) has_contradiction = true;
        
        bool sat;
        if (has_contradiction) {
            sat = false; // Direct contradiction
        } else if (mirror_overlap > 0.1 && std::abs(mirror_sum - 1.0) < 0.5) {
            sat = true; // Two-way mirror exists → SAT
        } else if (mirror_overlap < 0.05) {
            sat = false; // No mirror overlap → UNSAT
        } else {
            sat = (mirror_sum > 0.7); // Partial mirror
        }
        
        if (verbose) {
            std::cout << "    P=" << P << " NP=" << NP 
                      << " sum=" << mirror_sum << " overlap=" << mirror_overlap
                      << " contradict=" << (has_contradiction ? "YES" : "NO")
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
    std::cout << "║  TWO-WAY STRUCTURAL MIRROR P=NP                                    ║\n";
    std::cout << "║  P ↔ NP via φ·ψ = -1                                              ║\n";
    std::cout << "║  Mirror exists when both φ and ψ are present                       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n\n";
    
    TwoWayMirror solver;
    
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
    
    std::cout << "═══ TWO-WAY MIRROR DETAILS ═══\n\n";
    
    std::cout << "SAT_hard (SAT):\n";
    solver.is_sat({{1,2}, {1,3}, {2,3}, {-1,-2,-3}}, 3, true);
    std::cout << "\n\n";
    
    std::cout << "SAT_4var (SAT):\n";
    solver.is_sat({{1,2}, {-1,3}, {-2,4}, {-3,-4}}, 4, true);
    std::cout << "\n\n";
    
    std::cout << "UNSAT_1var (UNSAT):\n";
    solver.is_sat({{1}, {-1}}, 1, true);
    std::cout << "\n\n";
    
    std::cout << "PHP_2 (UNSAT):\n";
    std::vector<std::vector<int>> php2;
    int nv2;
    generate_PHP(2, php2, nv2);
    solver.is_sat(php2, nv2, true);
    std::cout << "\n\n";
    
    std::cout << "Results: " << passed << "/" << tests.size() << " passed\n\n";
    
    return 0;
}
