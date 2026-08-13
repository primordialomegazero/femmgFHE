#include "src/np/golden_sat.h"
#include <iostream>

int main() {
    std::cout << "=== SAT DEBUG ===\n\n";

    std::vector<std::vector<int>> clauses = {
        {1, 2},
        {-1, 2},
        {1, -2}
    };

    std::vector<bool> assignment = {true, true};

    double score = GoldenSAT::GoldenSATSolver::golden_score(clauses, assignment);
    std::cout << "Score for (1,1): " << score << "\n";

    assignment = {false, false};
    score = GoldenSAT::GoldenSATSolver::golden_score(clauses, assignment);
    std::cout << "Score for (0,0): " << score << "\n";

    assignment = {true, false};
    score = GoldenSAT::GoldenSATSolver::golden_score(clauses, assignment);
    std::cout << "Score for (1,0): " << score << "\n";

    assignment = {false, true};
    score = GoldenSAT::GoldenSATSolver::golden_score(clauses, assignment);
    std::cout << "Score for (0,1): " << score << "\n";

    return 0;
}
