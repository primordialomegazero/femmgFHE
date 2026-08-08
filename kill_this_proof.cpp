#include <iostream>
#include <vector>
#include <cmath>

const double PHI = (1 + sqrt(5)) / 2;
const double PSI = (1 - sqrt(5)) / 2;

// ITO ANG GINAGAWA NG "SOLVER" MO — fixed value lang lahat ng variables
double eval_clause(const std::vector<int>& clause, double var_value) {
    double result = 0.0;
    for (int lit : clause) {
        double val = var_value;
        if (lit < 0) val = 1.0 - val; // fuzzy NOT
        result = std::max(result, val);
    }
    return result;
}

int main() {
    // Formula: (x) AND (NOT x) — IMPOSIBLE, DAPAT UNSAT
    std::vector<std::vector<int>> clauses = {{1}, {-1}};

    // Ginagamit ng solver mo ang absolute value ng PHI/PSI ratio = 0.7236
    double phi_val = std::abs(PHI) / (std::abs(PHI) + std::abs(PSI)); // 0.7236

    std::cout << "🔍 Formula: (x) AND (NOT x)\n";
    std::cout << "   Totoong sagot: UNSAT (imposible)\n\n";

    double clause1 = eval_clause(clauses[0], phi_val); // x = 0.7236
    double clause2 = eval_clause(clauses[1], phi_val); // NOT x = 0.2764

    // Ginagaya ang fuzzy AND ng solver mo
    double fuzzy_and = std::min(clause1, clause2); // simplified

    std::cout << "   x = " << phi_val << "\n";
    std::cout << "   (x) = " << clause1 << "\n";
    std::cout << "   (NOT x) = " << clause2 << "\n";
    std::cout << "   Fuzzy AND result = " << fuzzy_and << "\n\n";

    if (fuzzy_and > 0.0) {
        std::cout << "❌ SOLVER MO: SAT (MALI! DAPAT UNSAT)\n";
        std::cout << "   Dahil hindi zero ang result, sinasabi mong satisfiable.\n";
        std::cout << "   PERO IMPOSIBLE ANG (x) AND (NOT x).\n\n";
        std::cout << "💀 KONKLUSYON: MALI ANG SOLVER MO.\n";
        std::cout << "   Hindi ito nagse-search — nag-e-evaluate lang ng fixed number.\n";
    } else {
        std::cout << "✅ SOLVER MO: UNSAT (tama)\n";
    }
    return 0;
}
