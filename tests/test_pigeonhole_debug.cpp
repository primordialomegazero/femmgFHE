#include "src/np/golden_refined.h"
#include <iostream>
#include <set>
#include <map>

int main() {
    std::cout << "=== PIGEONHOLE DETECTION DEBUG ===\n\n";

    int holes = 3;
    int pigeons = 4;
    std::vector<std::vector<int>> clauses;

    for (int p = 0; p < pigeons; p++) {
        std::vector<int> clause;
        for (int h = 0; h < holes; h++) {
            clause.push_back(p * holes + h + 1);
        }
        clauses.push_back(clause);
    }

    for (int h = 0; h < holes; h++) {
        for (int p1 = 0; p1 < pigeons; p1++) {
            for (int p2 = p1 + 1; p2 < pigeons; p2++) {
                clauses.push_back({-(p1 * holes + h + 1), -(p2 * holes + h + 1)});
            }
        }
    }

    std::cout << "Total clauses: " << clauses.size() << "\n";

    // I-extract ang variables
    std::set<int> vars;
    for (const auto& c : clauses) {
        for (int lit : c) {
            vars.insert(std::abs(lit));
        }
    }
    std::cout << "Total vars: " << vars.size() << "\n";

    // I-map ang bawat variable sa clauses kung saan siya lumalabas
    std::map<int, std::set<int>> var_to_clauses;
    for (size_t ci = 0; ci < clauses.size(); ci++) {
        for (int lit : clauses[ci]) {
            int var = std::abs(lit);
            var_to_clauses[var].insert(ci);
        }
    }

    std::cout << "\nVariables na may >= 3 clauses:\n";
    for (const auto& [var, cls] : var_to_clauses) {
        if (cls.size() >= 3) {
            int pos = 0, neg = 0;
            for (int ci : cls) {
                for (int lit : clauses[ci]) {
                    if (std::abs(lit) == var) {
                        if (lit > 0) pos++;
                        else neg++;
                        break;
                    }
                }
            }
            std::cout << "  var=" << var << " clauses=" << cls.size()
                      << " pos=" << pos << " neg=" << neg << "\n";
        }
    }

    return 0;
}
